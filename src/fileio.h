#ifndef ENDIAN_FILEIO_H_
#define ENDIAN_FILEIO_H_

#include <stdio.h>

#define JSMN_HEADER
#include <concord/jsmn.h>

// Moves all contents of file into a buffer. buf must be freed after use
char* fileio_read_all(FILE* file);

jsmntok_t* fileio_read_json(const char* json);

// returns 0 if ok
int fileio_ensure_dir_exists(const char* path);

#define dir_load(directory_arg, function_arg)                                  \
  /*                                                                           \
    calls 'function_arg' for every top level file or directory in              \
    'directory_arg' (which is relative to file system root)                    \
    .                                                                          \
    parameters:                                                                \
    * directory_arg    - const char*                                           \
    * function_arg     - expression                                            \
    .                                                                          \
    these variables are available to be used as arguments in the function call \
    for 'function_arg':                                                        \
    * file_name        - const char*                                           \
    * file_path        - const char*                                           \
  */                                                                           \
  do {                                                                         \
    const char* _dir = directory_arg;                                          \
                                                                               \
    DIR* _dir_handle = opendir(_dir);                                          \
    struct dirent* _dirent;                                                    \
                                                                               \
    if (!_dir_handle) {                                                        \
      log_error("Could not open folder at %s", _dir);                          \
      break;                                                                   \
    }                                                                          \
                                                                               \
    while ((_dirent = readdir(_dir_handle)) != NULL) {                         \
      if (_dirent->d_name[0] == '.') {                                         \
        continue;                                                              \
      }                                                                        \
      const char* file_name = _dirent->d_name;                                 \
      char* _file_path = malloc(strlen(_dir) + 1 + strlen(file_name) + 1);     \
      strcpy(_file_path, _dir);                                                \
      strcat(_file_path, "/");                                                 \
      strcat(_file_path, file_name);                                           \
      const char* file_path = _file_path;                                      \
      function_arg;                                                            \
      free(_file_path);                                                        \
    }                                                                          \
    closedir(_dir_handle);                                                     \
  } while (0)

#endif
