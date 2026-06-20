#ifndef ENDIAN_COMMAND_H_
#define ENDIAN_COMMAND_H_

#include <concord/discord.h>
#include <concord/discord_codecs.h>

#include "fid.h"

struct command_options {
  int size;
  struct command_option* options;
};

struct command_option {
  bool required;
  bool autocomplete;
  enum discord_application_command_option_types type;
  char* name;
  char* description;
  struct discord_application_command_option_choices* choices;
  struct command_options* options;
  struct integers* channel_types;
  char* min_value;
  char* max_value;
};

struct command {
  enum discord_application_command_types type;
  char* name;
  char* description;
  const struct fid* callback;
  struct command_options* options;
  unsigned long default_member_permissions;
};

void command_load(const struct discord_ready* event, const char* command_path,
                  const char* mod_name, const char* file_name);
const struct command* command_get(char* name);

int command_cmp(const struct command* a, const struct command* b);

void command_cleanup(struct command* elem);

#endif
