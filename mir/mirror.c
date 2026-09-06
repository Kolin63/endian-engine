#include "mirror.h"

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#define JSMN_HEADER
#include <log.h>
#include <concord/jsmn.h>

#include "jsmn_iterator.h"
#include "json_macros.h"
#include "mod_stack.h"
#include "../src/fileio.h"

static struct mirrors global = {};

ENDVEC_DEFINE(mirror_format_tokens, struct mirror_format_token, mirror_format_token_cleanup(arr->arr + i));
ENDVEC_DEFINE(mirror_files, struct mirror_file, mirror_file_cleanup(arr->arr + i));
ENDVEC_DEFINE(mirrors, struct mirror, mirror_cleanup(arr->arr + i));

void
mirror_format_token_cleanup(struct mirror_format_token* f) {
  if (f == NULL) return;
  if (f->buf != NULL) free(f->buf);
}

int
mirror_format_tokens_from_line(struct mirror_format_tokens* f, const char* _line) {
  int error = 0;

  char* line = malloc(strlen(_line) + 2);
  strcpy(line, _line);
  strcat(line, "\n");

  struct mirror_format_token* current_token = NULL;

  bool escape = false;

  const size_t linelen = strlen(line);
  for (size_t i = 0; i < linelen; i++) {
    if (line[i] == '%') {
      if (escape == true) {
        escape = false;
      } else {
        escape = true;
        continue;
      }
    }

    if (escape == true) {
      escape = false;
      enum mirror_format_token_type type;
      switch (line[i]) {
      case 't':
        type = MIR_TAG_CONTENT;
        break;
      case 'd':
        type = MIR_DATA;
        break;
      case 'D':
        type = MIR_DATA_CAPS;
        break;
      case 'n':
        type = MIR_NS;
        break;
      case 'N':
        type = MIR_NS_CAPS;
        break;
      case 'A':
        type = MIR_ALPHA_SWITCH;
        break;
      default:
        log_error(MOD_STACK_FMT "unknown escape sequence %%%c", MOD_STACK_ARG, line[i]);
        error++;
        continue;
      }
      mirror_format_tokens_append(f, (struct mirror_format_token){.type = type});
      current_token = &f->arr[f->len - 1];

      continue;
    }

    // at this point we are sure the text is of type CONST

    if (current_token == NULL || current_token->type != MIR_CONST) {
      mirror_format_tokens_append(f, (struct mirror_format_token){.type = MIR_CONST});
      current_token = &f->arr[f->len - 1];
    }

    if (current_token->buf == NULL) {
      current_token->buf = malloc(linelen + 1);
      current_token->buf[0] = '\0';
    }

    char charstr[2] = {line[i], '\0'};
    strcat(current_token->buf, charstr);
  }

  free(line);

  return error;
}

int
mirror_format_tokens_foreach_line_from_json(struct mirror_format_tokens* f, char* tag, const jsmntok_t* jsmn, const char* json) {
  int error = 0;

  mirror_format_tokens_append(f, (struct mirror_format_token){.type = MIR_FOREACH_BEGIN, .buf = tag});

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  while (jsmn_iterator_next(&iter)) {
    END_JSON_CHECK_STRING(iter);
    char* line = jsmn_iterator_get_string_heap(json, iter.val);
    error += mirror_format_tokens_from_line(f, line);
    free(line);
  }

  mirror_format_tokens_append(f, (struct mirror_format_token){.type = MIR_FOREACH_END});

  return error;
}

int
mirror_format_tokens_foreach_from_json(struct mirror_format_tokens* f, const jsmntok_t* jsmn, const char* json) {
  int error = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  char* tag = NULL;
  const jsmntok_t* format_arr = NULL;

  while (jsmn_iterator_next(&iter)) {
    if (strcmp(iter.key, "tag") == 0) {
      END_JSON_CHECK_STRING(iter);
      tag = jsmn_iterator_get_string_heap(json, iter.val);
    } else if (strcmp(iter.key, "format") == 0) {
      END_JSON_CHECK_ARRAY(iter);
      format_arr = iter.val;
    } else {
      log_error(MOD_STACK_FMT "unknown foreach key %s", MOD_STACK_ARG, iter.key);
      error++;
    }
  }

  if (format_arr != NULL) {
    error += mirror_format_tokens_foreach_line_from_json(f, tag, format_arr, json);
  }

  return error;
}

int
mirror_format_tokens_from_json(struct mirror_format_tokens* f, const jsmntok_t* jsmn, const char* json) {
  int error = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  while (jsmn_iterator_next(&iter)) {
    if (iter.val->type == JSMN_STRING) {
      END_JSON_CHECK_STRING(iter);
      char* line = jsmn_iterator_get_string_heap(json, iter.val);
      error += mirror_format_tokens_from_line(f, line);
      free(line);
    } else if (iter.val->type == JSMN_OBJECT) {
      END_JSON_CHECK_OBJECT(iter);
      error += mirror_format_tokens_foreach_from_json(f, iter.val, json);
    } else {
      log_error(MOD_STACK_FMT "format tokens must either be STRING or OBJECT", MOD_STACK_ARG);
      error++;
    }
  }

  return error;
}

void
mirror_file_cleanup(struct mirror_file* f) {
  if (f == NULL) return;
  free(f->name);
  mirror_format_tokens_cleanup(&f->tokens);
}

int
mirror_file_from_json(struct mirror_file* f, const jsmntok_t* jsmn, const char* json) {
  int error = 0;

  f->name = NULL;
  f->tokens.arr = NULL;
  f->tokens.len = 0;
  f->tokens.cap = 0;

  mirror_format_tokens_append(
      &f->tokens,
      (struct mirror_format_token){
          .type = MIR_CONST,
          .buf = strdup("// THIS FILE IS AUTOGENERATED BY THE REFLECTOR. DO NOT EDIT\n\n"),
      });

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_OBJECT_RET(iter, error++; return error);

  while (jsmn_iterator_next(&iter)) {
    if (strcmp(iter.key, "name") == 0) {
      END_JSON_CHECK_STRING(iter);
      f->name = jsmn_iterator_get_string_heap(json, iter.val);
    } else if (strcmp(iter.key, "format") == 0) {
      END_JSON_CHECK_ARRAY(iter);
      error += mirror_format_tokens_from_json(&f->tokens, iter.val, json);
    } else {
      error++;
      log_error(MOD_STACK_FMT "Unknown object %s", MOD_STACK_ARG, iter.key);
    }
  }

  return error;
}

int
mirror_files_from_json(struct mirror_files* arr, const jsmntok_t* jsmn, const char* json) {
  int error = 0;

  arr->arr = NULL;
  arr->len = 0;
  arr->cap = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_ARRAY_RET(iter, error++; return error);

  while (jsmn_iterator_next(&iter)) {
    mirror_files_append(arr, (struct mirror_file){});
    error += mirror_file_from_json(&arr->arr[arr->len - 1], iter.val, json);
  }

  return error;
}

void
mirror_cleanup(struct mirror* m) {
  if (m == NULL) return;
  if (m->id != NULL) free(m->id);
  mirror_files_cleanup(&m->files);
}

int
mirror_from_json(struct mirror* m, const jsmntok_t* jsmn, const char* json) {
  int error = 0;

  m->id = NULL;
  m->files.arr = NULL;
  m->files.len = 0;
  m->files.cap = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_OBJECT_RET(iter, error++; return error);

  while (jsmn_iterator_next(&iter)) {
    if (strcmp(iter.key, "id") == 0) {
      END_JSON_CHECK_STRING(iter);
      m->id = jsmn_iterator_get_string_heap(json, iter.val);
    } else if (strcmp(iter.key, "files") == 0) {
      END_JSON_CHECK_ARRAY(iter);
      error += mirror_files_from_json(&m->files, iter.val, json);
    } else {
      error++;
      log_error(MOD_STACK_FMT "Unknown object %s", MOD_STACK_ARG, iter.key);
    }
  }

  return error;
}

void
mirror_load(const char* file_path) {
  if (strcmp(mod_stack_global()->file, "template.json") == 0) return;

  FILE* file = fopen(file_path, "r");

  if (!file) {
    log_error(MOD_STACK_FMT "Could not open mirror file (%s)", MOD_STACK_ARG, file_path);
    return;
  }

  char* json = fileio_read_all(file);
  fclose(file);

  mirror_load_from_str(json);
  free(json);
}

void
mirror_load_from_str(const char* str) {
  jsmntok_t* jsmn = fileio_read_json(str);

  struct mirror m;
  if (mirror_from_json(&m, jsmn, str) != 0) {
    free(jsmn);
    return;
  }

  free(jsmn);

  mirrors_append(&global, m);

  log_info("Loading mirror %s", m.id);
}

struct mirrors*
mirrors_global() {
  return &global;
}
