#ifndef ENDIAN_SAVE_H_
#define ENDIAN_SAVE_H_

#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "bot.h"
#include "namespace.h"

#ifdef ENDIAN_ENGINE

// writes to save file. returns 0 if ok.
// dir does not need a trailing slash
// ext is file extension, and it should not include the dot. for example, a
// json file has the extension "json", not ".json"
int save_write(const char* dir, const char* file, const char* ext, const char* content);

// reads from save file into out. returns 0 if ok
// dir does not need a trailing slash
// ext is file extension, and it should not include the dot. for example, a
// json file has the extension "json", not ".json"
int save_read(const char* dir, const char* file, const char* ext, char** out);

#endif

// calls 'function_arg' for every top level file or directory in 'directory_arg'
// for every namespace 'namespace_arg' in all loaded mods
//
// parameters:
// * namespace_arg    - const char*
// * directory_arg    - const char*
// * function_arg     - expression
//
// these variables are available to be used as arguments in the function call
// for 'function_arg':
// * file_name        - const char*
// * file_path        - const char*
#define rom_load(namespace_arg, directory_arg, function_arg)                                            \
  do {                                                                                                  \
    const char* _ns = namespace_arg;                                                                    \
    const char* _dir = directory_arg;                                                                   \
    const struct namespace* _cache = namespace_get(_ns);                                                \
                                                                                                        \
    char* _mods_path = malloc(strlen(bot_get_global()->instance_dir) + 6 + 1);                          \
    strcpy(_mods_path, bot_get_global()->instance_dir);                                                 \
    strcat(_mods_path, "/mods/");                                                                       \
                                                                                                        \
    for (int _i = 0; _i < _cache->mods->length; _i++) {                                                 \
      const struct namespace_mod_entry* _modent = registry_itov(_cache->mods, _i);                      \
      char* _rom_path = malloc(strlen(_mods_path) + strlen(_modent->name) + 17 + strlen(_dir) + 1 + 1); \
      strcpy(_rom_path, _mods_path);                                                                    \
      strcat(_rom_path, _modent->name);                                                                 \
      strcat(_rom_path, "/data/endian/rom/");                                                           \
      strcat(_rom_path, _dir);                                                                          \
      strcat(_rom_path, "/");                                                                           \
                                                                                                        \
      DIR* _dir_handle = opendir(_rom_path);                                                            \
      struct dirent* _dirent;                                                                           \
                                                                                                        \
      if (!_dir_handle) {                                                                               \
        log_error("Could not open folder at %s", _rom_path);                                            \
        break;                                                                                          \
      }                                                                                                 \
                                                                                                        \
      while ((_dirent = readdir(_dir_handle)) != NULL) {                                                \
        if (_dirent->d_name[0] == '.') {                                                                \
          continue;                                                                                     \
        }                                                                                               \
        const char* file_name = _dirent->d_name;                                                        \
        char* _file_path = malloc(strlen(_rom_path) + strlen(file_name) + 1);                           \
        strcpy(_file_path, _rom_path);                                                                  \
        strcat(_file_path, file_name);                                                                  \
        const char* file_path = _file_path;                                                             \
        function_arg;                                                                                   \
        free(_file_path);                                                                               \
      }                                                                                                 \
      closedir(_dir_handle);                                                                            \
      free(_rom_path);                                                                                  \
    }                                                                                                   \
    free(_mods_path);                                                                                   \
  } while (0)

#endif
