// clang-format off

#define ENDIAN_CORE_MIRRORS_FUNCTIONS "{\
  \"id\": \"functions\",\
  \"files\": [\
    {\
      \"name\": \"functions.h\",\
      \"groups\": [\
        {\
          \"prefix\": [\
            \"#ifndef ENDIAN_REFLECTIONS_FUNCTIONS_H_\",\
            \"#define ENDIAN_REFLECTIONS_FUNCTIONS_H_\",\
            \"\",\
          ],\
          \"foreach\": [\
            {\
              \"tag\": \"PREPROC\",\
              \"format\": [\
                \"%t\"\
              ]\
            }\
          ],\
          \"postfix\": [\
            \"#include \\\"../src/function.h\\\"\",\
            \"\"\
          ]\
        },\
\
\
        {\
          \"prefix\": [\
            \"#endif\"\
          ]\
        }\
      ]\
    },\
\
\
\
\
\
    {\
      \"name\": \"functions.c\",\
      \"groups\": [\
        {\
          \"prefix\": [\
            \"#include \\\"functions.h\\\"\",\
            \"\"\
          ]\
        },\
\
\
        {\
          \"prefix\": [\
            \"void\",\
            \"function_call_init() {\"\
          ],\
          \"foreach\": [\
            {\
              \"tag\": \"function_init\",\
              \"format\": [\
                \"{%t}\"\
              ]\
            }\
          ],\
          \"postfix\": [\
            \"}\",\
            \"\"\
          ]\
        },\
\
\
        {\
          \"prefix\": [\
            \"void\",\
            \"function_call_load() {\"\
          ],\
          \"foreach\": [\
            {\
              \"tag\": \"function_load\",\
              \"format\": [\
                \"{%t}\"\
              ]\
            }\
          ],\
          \"postfix\": [\
            \"}\",\
            \"\"\
          ]\
        },\
\
\
        {\
          \"prefix\": [\
            \"void\",\
            \"function_call_save() {\"\
          ],\
          \"foreach\": [\
            {\
              \"tag\": \"function_save\",\
              \"format\": [\
                \"{%t}\"\
              ]\
            }\
          ],\
          \"postfix\": [\
            \"}\",\
            \"\"\
          ]\
        },\
\
\
        {\
          \"prefix\": [\
            \"void\",\
            \"function_call_cleanup() {\"\
          ],\
          \"foreach\": [\
            {\
              \"tag\": \"function_cleanup\",\
              \"format\": [\
                \"{%t}\"\
              ]\
            }\
          ],\
          \"postfix\": [\
            \"}\"\
          ]\
        }\
      ]\
    }\
  ]\
}"

#define ENDIAN_CORE_MIRRORS_COMMANDS "{\
  \"id\": \"commands\",\
  \"files\": [\
    {\
      \"name\": \"commands.h\",\
      \"groups\": [\
        {\
          \"prefix\": [\
            \"#ifndef ENDIAN_REFLECTIONS_COMMANDS_H_\",\
            \"#define ENDIAN_REFLECTIONS_COMMANDS_H_\",\
            \"\",\
          ],\
          \"foreach\": [\
            {\
              \"tag\": \"PREPROC\",\
              \"format\": [\
                \"%t\"\
              ]\
            }\
          ],\
          \"postfix\": [\
            \"#include \\\"../src/command.h\\\"\",\
            \"\"\
          ]\
        },\
\
\
        {\
          \"prefix\": [\
            \"enum command_id {\",\
            \"  COMMAND_UNKNOWN_,\"\
          ],\
          \"foreach\": [\
            {\
              \"tag\": \"command\",\
              \"format\": [\
                \"  COMMAND_%D,\"\
              ]\
            }\
          ],\
          \"postfix\": [\
            \"};\",\
            \"\"\
          ]\
        },\
\
\
        {\
          \"prefix\": [\
            \"const static struct command commands[] = {\",\
            \"\"\
          ],\
          \"foreach\": [\
            {\
              \"tag\": \"command\",\
              \"format\": [\
                \"[COMMAND_%D] = %t,\"\
              ]\
            }\
          ],\
          \"postfix\": [\
            \"};\",\
            \"\"\
          ]\
        },\
\
\
        {\
          \"foreach\": [\
            {\
              \"tag\": \"command_cb\",\
              \"format\": [\
                \"void %n_%d_cb(struct discord* client, const struct discord_interaction* event);\",\
                \"\"\
              ]\
            }\
          ]\
        },\
\
\
        {\
          \"prefix\": [\
            \"#endif\"\
          ]\
        }\
      ]\
    },\
\
\
\
\
\
    {\
      \"name\": \"commands.c\",\
      \"groups\": [\
        {\
          \"prefix\": [\
            \"#include \\\"commands.h\\\"\",\
            \"\"\
          ]\
        },\
\
\
        {\
          \"foreach\": [\
            {\
              \"tag\": \"command_cb\",\
              \"format\": [\
                \"void\",\
                \"%n_%d_cb(struct discord* client, const struct discord_interaction* event) %t\"\
              ]\
            }\
          ]\
        },\
\
\
        {\
          \"prefix\": [\
            \"enum command_id\",\
            \"command_id_get(const char* str) {\"\
          ],\
          \"foreach\": [\
            {\
              \"tag\": \"command\",\
              \"format\": [\
                \"%A\",\
                \"// %d\",\
                \"%A\",\
                \"    // return COMMAND_%D\",\
                \"%A\",\
                \"    // return COMMAND_UNKNOWN_;\",\
                \"%A\"\
              ]\
            }\
          ],\
          \"postfix\": [\
            \"}\",\
            \"\"\
          ]\
        },\
\
\
        {\
          \"prefix\": [\
            \"void\",\
            \"command_cb(enum command_id id, struct discord* client, const struct discord_interaction* event) {\",\
            \"  switch (id) {\"\
          ],\
          \"foreach\": [\
            {\
              \"tag\": \"command_cb\",\
              \"format\": [\
                \"  case COMMAND_%D: %n_%d_cb(client, event); return;\"\
              ]\
            }\
          ],\
          \"postfix\": [\
            \"  }\",\
            \"}\",\
            \"\"\
          ]\
        },\
\
\
        {\
          \"prefix\": [\
            \"void\",\
            \"command_create_all(const struct discord_ready* event) {\"\
          ],\
          \"foreach\": [\
            {\
              \"tag\": \"command\",\
              \"format\": [\
                \"  command_create(&commands[COMMAND_%D], event);\"\
              ]\
            }\
          ],\
          \"postfix\": [\
            \"}\"\
          ]\
        }\
      ]\
    }\
  ]\
}"
