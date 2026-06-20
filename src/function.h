#ifndef ENDIAN_FUNCTION_H_
#define ENDIAN_FUNCTION_H_

#include "fid.h"

#include <concord/discord.h>

enum function_type {
  FT_CALLBACK,
  FT_GET_API,
  FT_INIT,
  FT_LOAD,
  FT_FILLOUT,
  FT_EXPORT,
  FT_TO_JSON,
  FT_DATA,
  FT_SAVE,
  FT_CLEANUP,
};

struct function {
  enum function_type type;
  const void (*function)(...);
  char* name;
  const struct fid* plugin_fid;
};

#ifdef ENDIAN_ENGINE

void function_load(const char* function_path, const char* namespace_name, const char* mod_name, const char* file_name);
const struct function* function_get(char* name);

int function_cmp(const struct function* a, const struct function* b);

void function_cleanup(struct function* elem);

#endif

// calls a function, which must be of type EXPORT. sets error to 0 if ok.
#define function_call(name, error, ...)                 \
  do {                                                  \
    const struct function* func = function_get(name);   \
    if (func == NULL) {                                 \
      log_error("Function %s is not registered", name); \
      error = -1;                                       \
      break;                                            \
    }                                                   \
    if (func->function == NULL) {                       \
      log_error("Handle of function %s is NULL", name); \
      error = -2;                                       \
      break;                                            \
    }                                                   \
    if (func->type != EXPORT) {                         \
      log_error("Function %s is not an EXPORT", name);  \
      error = -3;                                       \
      break;                                            \
    }                                                   \
                                                        \
    func->function(__VA_ARGS__);                        \
                                                        \
    error = 0;                                          \
  } while (0)

#endif
