#ifndef ENDIAN_GET_COMMAND_OPTIONS_H_
#define ENDIAN_GET_COMMAND_OPTIONS_H_

#include <stddef.h>
#include <string.h>

#include <concord/discord.h>
#include <concord/discord_codecs.h>

static inline bool
_get_cmd_opts_set(char** var, char* str) {
  *var = str;
  return true;
}

#define GET_COMMAND_OPTIONS(_discord_interaction)                               \
  if (_discord_interaction->data->options != NULL)                              \
    for (char *i = 0, *value = NULL, *_inter_cpy = (void*)_discord_interaction; \
         (size_t)i < _discord_interaction->data->options->size;                 \
         i++)

// clang-format off
#define GET_COMMAND_OPTION(option_name) \
  if ( \
      strcmp( \
        ((struct discord_interaction*)_inter_cpy)->data->options->array[(size_t)i].name, \
        option_name \
      ) == 0 \
      && \
      _get_cmd_opts_set( \
        &value, \
        ((struct discord_interaction*)_inter_cpy)->data->options->array[(size_t)i].value \
      ) == true \
    )
// clang-format on

#endif
