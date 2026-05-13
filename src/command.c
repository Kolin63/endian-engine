#include "command.h"

#include <assert.h>
#include <concord/application_command.h>
#include <concord/discord.h>
#include <concord/discord_codecs.h>
#include <concord/jsmn.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define JSMN_HEADER
#include <concord/jsmn.h>

#include "bot.h"
#include "command.h"
#include "fileio.h"
#include "function.h"
#include "json_macros.h"
#include "log.h"
#include "registry.h"
#include "regman.h"
#include "jsmn_iterator.h"

// converts endian command options to concord command options
struct discord_application_command_options* command_options_end_to_conc(const struct command_options* end) {
  if (end == NULL) return NULL;

  struct discord_application_command_options* conc = malloc(sizeof(struct discord_application_command_options));
  conc->size = end->size;
  conc->array = malloc(sizeof(struct discord_application_command_option) * conc->size);
  for (int i = 0; i < end->size; i++) {
    const struct command_option* eopt = &(end->options[i]);
    struct discord_application_command_option* copt = &(conc->array[i]);
    copt->required = eopt->required;
    copt->autocomplete = eopt->autocomplete;
    copt->type = eopt->type;
    copt->name = eopt->name;
    copt->description = eopt->description;
    copt->choices = eopt->choices;
    copt->channel_types = eopt->channel_types;
    copt->min_value = eopt->min_value;
    copt->max_value = eopt->max_value;

    copt->options = command_options_end_to_conc(eopt->options);
  }
  return conc;
}

void discord_application_command_options_cleanup(struct discord_application_command_options* opts) {
  if (opts == NULL) return;
  for (int i = 0; i < opts->size; i++) {
    discord_application_command_options_cleanup(opts->array[i].options);
  }
  free(opts->array);
  free(opts);
}

// returns amount of errors, 0 if ok
// jsmn should point to the root of the choices array
int command_option_choices_fillout(const char* mod_name, const char* file_name,
                                   const jsmntok_t* jsmn, const char* json,
                                   struct discord_application_command_option_choices* choices) {
  int error = 0;

  struct jsmn_iterator arr_iter = {};
  jsmn_iterator_init(&arr_iter, jsmn, json);

  END_JSON_CHECK_ARRAY_AND_CHILDREN_RET(arr_iter, return error);

  /* a choice is like this:
     {
       "name": "Foo",
       "value": "foo"
     }
   */

  while (jsmn_iterator_next(&arr_iter)) {
    // one choice is an object. going through the array, if there is a
    // non-object item, error
    END_JSON_CHECK_OBJECT_AND_CHILDREN(arr_iter);

    choices->size++;
    choices->array = reallocarray(choices->array, choices->size,
                                  sizeof(struct discord_application_command_option_choice));

    struct discord_application_command_option_choice* i = &(choices->array[choices->size - 1]);
    i->name = NULL;
    i->value = NULL;

    // now iterate through the values of the choice object
    struct jsmn_iterator obj_iter;
    jsmn_iterator_init(&obj_iter, arr_iter.val, json);

    while (jsmn_iterator_next(&obj_iter)) {
      END_JSON_CHECK_STRING(obj_iter);
      END_JSON_CHECK_STRING_LENGTH(obj_iter, 1, 100);

      struct discord_application_command_option_choice* i = &(choices->array[choices->size - 1]);
      if (i->name != NULL || i->value != NULL) {
        log_error("In command %s from mod %s, choices is wrong size", file_name, mod_name);
        error++;
        continue;
      }

      if (strcmp(obj_iter.key, "name") == 0) {
        i->name = jsmn_iterator_get_string_heap(json, obj_iter.val);
      } else if (strcmp(obj_iter.key, "value") == 0) {
        i->value = jsmn_iterator_get_string_heap(json, obj_iter.val);
      } else {
        log_error("Command choice from command %s from mod %s has unknown object %s",
                  file_name, mod_name, obj_iter.key);
        error++;
        continue;
      }
    }
  }
  return error;
}

// returns amount of errors, 0 if ok
// jsmn should point to the root of the channel_types array
int command_option_channel_types_fillout(const char* mod_name,
                                         const char* file_name,
                                         const jsmntok_t* jsmn,
                                         const char* json,
                                         struct integers* channel_types) {
  int error = 0;

  // iterates over array of strings
  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_ARRAY_AND_CHILDREN_RET(iter, return error);

  while (jsmn_iterator_next(&iter)) {
    END_JSON_CHECK_STRING(iter);
    END_JSON_CHECK_STRING_LENGTH(iter, 1, 100);

    char val[32];
    jsmn_iterator_get_string(val, 128, json, iter.val);
    int type;

    if (strcmp(val, "GUILD_TEXT") == 0) type = 0;
    else if (strcmp(val, "DM") == 0) type = 1;
    else if (strcmp(val, "GUILD_VOICE") == 0) type = 2;
    else if (strcmp(val, "GROUP_DM") == 0) type = 3;
    else if (strcmp(val, "GUILD_CATEGORY") == 0) type = 4;
    else if (strcmp(val, "GUILD_ANNOUNCEMENT") == 0) type = 5;
    else if (strcmp(val, "ANNOUNCEMENT_THREAD") == 0) type = 10;
    else if (strcmp(val, "PUBLIC_THREAD") == 0) type = 11;
    else if (strcmp(val, "PRIVATE_THREAD") == 0) type = 12;
    else if (strcmp(val, "GUILD_STAGE_VOICE") == 0) type = 13;
    else if (strcmp(val, "GUILD_DIRECTORY") == 0) type = 14;
    else if (strcmp(val, "GUILD_FORUM") == 0) type = 15;
    else if (strcmp(val, "GUILD_MEDIA") == 0) type = 16;
    else {
      log_error("In command %s from mod %s, unknown channel type %s", file_name, mod_name, val);
      error++;
      continue;
    }

    channel_types->size++;
    channel_types->array = reallocarray(channel_types->array, channel_types->size, sizeof(int));
    channel_types->array[channel_types->size - 1] = type;
  }

  return error;
}

// (forward declaration)
// returns amount of errors, 0 if ok
// jsmn should point to the root of the options array
int command_options_fillout(const char* mod_name, const char* file_name,
                            const jsmntok_t* jsmn, const char* json,
                            struct command_options* opts);

// returns amount of errors, 0 if ok
// jsmn should point to the root of the option
int command_option_fillout(const char* mod_name, const char* file_name,
                           const jsmntok_t* jsmn, const char* json,
                           struct command_option* opt) {
  int error = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_OBJECT_AND_CHILDREN_RET(iter, return error);

  while (jsmn_iterator_next(&iter)) {
    if (strcmp(iter.key, "type") == 0) {
      END_JSON_CHECK_STRING(iter);

      char val[32];
      jsmn_iterator_get_string(val, 32, json, iter.val);

      if (strcmp(val, "SUB_COMMAND") == 0) opt->type = DISCORD_APPLICATION_OPTION_SUB_COMMAND;
      else if (strcmp(val, "SUB_COMMAND_GROUP") == 0) opt->type = DISCORD_APPLICATION_OPTION_SUB_COMMAND_GROUP;
      else if (strcmp(val, "STRING") == 0) opt->type = DISCORD_APPLICATION_OPTION_STRING;
      else if (strcmp(val, "INTEGER") == 0) opt->type = DISCORD_APPLICATION_OPTION_INTEGER;
      else if (strcmp(val, "BOOLEAN") == 0) opt->type = DISCORD_APPLICATION_OPTION_BOOLEAN;
      else if (strcmp(val, "USER") == 0) opt->type = DISCORD_APPLICATION_OPTION_USER;
      else if (strcmp(val, "CHANNEL") == 0) opt->type = DISCORD_APPLICATION_OPTION_CHANNEL;
      else if (strcmp(val, "ROLE") == 0) opt->type = DISCORD_APPLICATION_OPTION_ROLE;
      else if (strcmp(val, "MENTIONABLE") == 0) opt->type = DISCORD_APPLICATION_OPTION_MENTIONABLE;
      else if (strcmp(val, "NUMBER") == 0) opt->type = DISCORD_APPLICATION_OPTION_NUMBER;
      else if (strcmp(val, "ATTACHMENT") == 0) opt->type = DISCORD_APPLICATION_OPTION_ATTACHMENT;
      else {
        log_error("In command %s from mod %s, unknown type %s", file_name, mod_name, val);
        error++;
        continue;
      }
    } else if (strcmp(iter.key, "name") == 0) {
      END_JSON_CHECK_STRING(iter);
      END_JSON_CHECK_STRING_LENGTH(iter, 1, 32);
      opt->name = jsmn_iterator_get_string_heap(json, iter.val);
    } else if (strcmp(iter.key, "description") == 0) {
      END_JSON_CHECK_STRING(iter);
      END_JSON_CHECK_STRING_LENGTH(iter, 1, 100);
      opt->description = jsmn_iterator_get_string_heap(json, iter.val);
    } else if (strcmp(iter.key, "required") == 0) {
      END_JSON_CHECK_BOOL(iter);
      char val[2];
      jsmn_iterator_get_string(val, 2, json, iter.val);
      opt->required = (val[0] == 't');
    } else if (strcmp(iter.key, "min_value") == 0) {
      END_JSON_CHECK_STRING(iter);
      opt->min_value = jsmn_iterator_get_string_heap(json, iter.val);
    } else if (strcmp(iter.key, "max_value") == 0) {
      END_JSON_CHECK_STRING(iter);
      opt->max_value = jsmn_iterator_get_string_heap(json, iter.val);
    } else if (strcmp(iter.key, "autocomplete") == 0) {
      END_JSON_CHECK_BOOL(iter);
      char val[2];
      jsmn_iterator_get_string(val, 2, json, iter.val);
      opt->autocomplete = (val[0] == 't');
    } else if (strcmp(iter.key, "options") == 0) {
      END_JSON_CHECK_ARRAY_AND_CHILDREN(iter);
      opt->options = malloc(sizeof(struct command_options));
      opt->options->options = NULL;
      opt->options->size = 0;
      error += command_options_fillout(mod_name, file_name, iter.val, json, opt->options);
    } else if (strcmp(iter.key, "choices") == 0) {
      END_JSON_CHECK_ARRAY_AND_CHILDREN(iter);
      opt->choices = malloc(sizeof(struct discord_application_command_option_choices));
      opt->choices->array = NULL;
      opt->choices->size = 0;
      error += command_option_choices_fillout(mod_name, file_name, iter.val, json, opt->choices);
    } else if (strcmp(iter.key, "channel_types") == 0) {
      END_JSON_CHECK_ARRAY_AND_CHILDREN(iter);
      opt->channel_types = malloc(sizeof(struct integers));
      opt->channel_types->array = NULL;
      opt->channel_types->size = 0;
      error += command_option_channel_types_fillout(mod_name, file_name, iter.val, json, opt->channel_types);
    } else {
      log_error("In command %s from mod %s, unknown object %s", file_name, mod_name, iter.key);
      error++;
      continue;
    }
  }

  return error;
}

// returns amount of errors, 0 if ok
// json should point to the root of the options array
int command_options_fillout(const char* mod_name, const char* file_name,
                            const jsmntok_t* jsmn, const char* json,
                            struct command_options* opts) {
  int error = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_ARRAY_AND_CHILDREN_RET(iter, return error);

  while (jsmn_iterator_next(&iter)) {
    if (iter.val->type != JSMN_OBJECT) {
      log_error("In option %s from mod %s, %s must be object", file_name, mod_name, iter.key);
      error++;
      continue;
    }
    if (iter.val->size == 0) {
      log_error("In option %s from mod %s, %s has no children", file_name, mod_name, iter.key);
      error++;
      continue;
    }

    struct command_option opt = {};
    int this_error = command_option_fillout(mod_name, file_name, iter.val, json, &opt);
    error += this_error;
    if (this_error == 0) {
      opts->size++;
      opts->options = reallocarray(opts->options, opts->size, sizeof(struct command_option));
      opts->options[opts->size - 1] = opt;
    }
  }

  return error;
}

// returns amount of errors, 0 if ok
int command_fillout(const char* mod_name, const char* file_name,
                    const jsmntok_t* jsmn, const char* json,
                    struct command* params) {
  int error = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_OBJECT_AND_CHILDREN_RET(iter, return error);

  while (jsmn_iterator_next(&iter)) {
    if (strcmp(iter.key, "type") == 0) {
      END_JSON_CHECK_STRING(iter);
      char val[16];
      jsmn_iterator_get_string(val, 16, json, iter.val);
      if (strcmp(val, "CHAT_INPUT") == 0) params->type = DISCORD_APPLICATION_CHAT_INPUT;
      else if (strcmp(val, "USER") == 0) params->type = DISCORD_APPLICATION_USER;
      else if (strcmp(val, "MESSAGE") == 0) params->type = DISCORD_APPLICATION_MESSAGE;
      else {
        log_error("In command %s from mod %s, unknown type %s", file_name, mod_name, val);
        error++;
        continue;
      }
    } else if (strcmp(iter.key, "name") == 0) {
      END_JSON_CHECK_STRING(iter);
      END_JSON_CHECK_STRING_LENGTH(iter, 1, 32);
      params->name = jsmn_iterator_get_string_heap(json, iter.val);
    } else if (strcmp(iter.key, "description") == 0) {
      END_JSON_CHECK_STRING(iter);
      END_JSON_CHECK_STRING_LENGTH(iter, 1, 100);
      params->description = jsmn_iterator_get_string_heap(json, iter.val);
    } else if (strcmp(iter.key, "default_member_permissions") == 0) {
      END_JSON_CHECK_NUMBER(iter);
      char val[32];
      jsmn_iterator_get_string(val, 32, json, iter.val);
      char* endptr = val;
      errno = 0;
      unsigned long perms = strtoul(val, &endptr, 10);
      if (errno != 0 || *endptr != '\0') {
        log_error("In command %s from mod %s, error reading default_member_permissions", file_name, mod_name);
        error++;
        continue;
      }
      params->default_member_permissions = perms;
    } else if (strcmp(iter.key, "callback") == 0) {
      END_JSON_CHECK_STRING(iter);
      char* func = jsmn_iterator_get_string_heap(json, iter.val);
      if (registry_ktoi(regman_get_function(), &(struct function){.name = func}) == -1) {
        log_error("In command %s from mod %s, function %s not registered", file_name, mod_name, func);
        error++;
        continue;
      }
      params->callback = func;
    } else if (strcmp(iter.key, "options") == 0) {
      END_JSON_CHECK_ARRAY_AND_CHILDREN(iter);
      params->options = malloc(sizeof(struct command_options));
      params->options->options = NULL;
      params->options->size = 0;
      command_options_fillout(mod_name, file_name, iter.val, json, params->options);
    } else {
      log_error("Command %s from mod %s has unknown object %s", file_name, mod_name, iter.key);
      error++;
      continue;
    }
  }

  return error;
}

void command_load(const struct discord_ready* event, const char* command_path,
                  const char* mod_name, const char* file_name) {
  if (strcmp(file_name, "template.json") == 0) return;

  FILE* file = fopen(command_path, "r");

  if (!file) {
    log_error("Could not open command file from mod %s at %s", mod_name, command_path);
    return;
  }

  char* json = fileio_read_all(file);
  fclose(file);

  jsmntok_t* jsmn = fileio_read_json(json);

  struct command params = {};
  if (command_fillout(mod_name, file_name, jsmn, json, &params) != 0) {
    free(json);
    free(jsmn);
    return;
  }

  free(json);
  free(jsmn);

  if (registry_add(regman_get_command(), &params) == NULL) {
    log_error("Command %s already registered", file_name);
    return;
  };

  struct discord_application_command_options* discord_opts = command_options_end_to_conc(params.options);

  struct discord_create_global_application_command discord_params = {
      .type = params.type,
      .name = params.name,
      .description = params.description,
      .options = discord_opts,
      .default_member_permissions = params.default_member_permissions};

  discord_create_global_application_command(bot_get_global()->discord_bot, event->application->id,
                                            &discord_params, NULL);
  discord_application_command_options_cleanup(discord_opts);
  log_info("Loading command %s from mod %s", file_name, mod_name);
}

const struct command* command_get(char* name) {
  return registry_ktov(regman_get_command(), &(struct command){.name = name});
}

void command_options_cleanup(struct command_options* opts) {
  if (opts == NULL) return;
  for (int i = 0; i < opts->size; i++) {
    struct command_option* opt = &(opts->options[i]);
    free(opt->name);
    free(opt->description);
    free(opt->min_value);
    free(opt->max_value);

    if (opt->choices != NULL) {
      for (int j = 0; j < opt->choices->size; j++) {
        struct discord_application_command_option_choice* c = &(opt->choices->array[j]);
        free(c->name);
        free(c->value);
      }
      free(opt->choices->array);
      free(opt->choices);
    }

    if (opt->channel_types != NULL) {
      free(opt->channel_types->array);
      free(opt->channel_types);
    }

    if (opt->options != NULL) {
      command_options_cleanup(opt->options);
    }
  }

  free(opts->options);
  free(opts);
}

int command_cmp(const struct command* a, const struct command* b) {
  return registry_strcmp(a->name, b->name);
}

void command_cleanup(struct command* elem) {
  free(elem->name);
  free(elem->description);
  free(elem->callback);
  command_options_cleanup(elem->options);
}
