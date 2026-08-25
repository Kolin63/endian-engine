#ifndef ENDIAN_COMMAND_H_
#define ENDIAN_COMMAND_H_

#include <concord/discord.h>
#include <concord/discord_codecs.h>

struct command_options {
  const int size;
  const struct command_option* options;
};

struct command_option {
  const bool required;
  const bool autocomplete;
  const enum discord_application_command_option_types type;
  const char* name;
  const char* description;
  const struct discord_application_command_option_choices choices;
  const struct command_options options;
  const struct integers channel_types;
  const char* min_value;
  const char* max_value;
};

struct command {
  const enum discord_application_command_types type;
  const char* name;
  const char* description;
  const struct command_options options;
  const unsigned long default_member_permissions;
};

enum command_id;

const static struct command commands[];

void command_cb(enum command_id id, struct discord* client, const struct discord_interaction* event);
void command_create(const struct command* cmd, const struct discord_ready* event);
void command_create_all(const struct discord_ready* event);
enum command_id command_id_get(const char* str);

#include "endapi/ref/commands.h"

#endif
