#include "command.h"

#include <concord/application_command.h>
#include <concord/discord.h>
#include <concord/discord_codecs.h>
#include <stdlib.h>
#include <string.h>

#include "bot.h"
#include "command.h"
#include "log.h"

void
discord_application_command_options_cleanup(struct discord_application_command_options* opts) {
  if (opts == NULL) return;
  for (int i = 0; i < opts->size; i++) {
    discord_application_command_options_cleanup(opts->array[i].options);
  }
  free(opts->array);
  free(opts);
}

// converts endian command options to concord command options
struct discord_application_command_options*
command_options_end_to_conc(const struct command_options* end) {
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
    copt->name = (char*)eopt->name;
    copt->description = (char*)eopt->description;
    copt->choices = (struct discord_application_command_option_choices*)&eopt->choices;
    copt->channel_types = (struct integers*)&eopt->channel_types;
    copt->min_value = (char*)eopt->min_value;
    copt->max_value = (char*)eopt->max_value;

    copt->options = command_options_end_to_conc(&eopt->options);
  }
  return conc;
}

void
command_create(const struct command* cmd, const struct discord_ready* event) {
  struct discord_application_command_options* discord_opts = command_options_end_to_conc(&cmd->options);

  struct discord_create_global_application_command discord_params = {
      .type = cmd->type,
      .name = (char*)cmd->name,
      .description = (char*)cmd->description,
      .options = discord_opts,
      .default_member_permissions = cmd->default_member_permissions};

  discord_create_global_application_command(bot_get_global()->discord_bot, event->application->id, &discord_params, NULL);
  discord_application_command_options_cleanup(discord_opts);
  log_info("Loading command %s", cmd->name);
}
