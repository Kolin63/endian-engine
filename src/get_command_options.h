#ifndef ENDIAN_GET_COMMAND_OPTIONS_H_
#define ENDIAN_GET_COMMAND_OPTIONS_H_

#include <string.h>

#include <concord/discord.h>
#include <concord/discord_codecs.h>

#define BEGIN_GET_COMMAND_OPTIONS(discord_interaction)        \
  if (event->data->options != NULL) {                         \
    for (int i = 0; i < event->data->options->size; i++) {    \
      const char* name = event->data->options->array[i].name; \
      char* value = event->data->options->array[i].value;     \
                                                              \
      if (false) {                                            \
      }

#define GET_COMMAND_OPTION(option_name) else if (strcmp(name, option_name) == 0)

#define END_GET_COMMAND_OPTIONS(discord_interaction) \
  }                                                  \
  }

#endif
