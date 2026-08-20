#ifndef ENDIAN_SAVE_H_
#define ENDIAN_SAVE_H_

#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "bot.h"
#include "namespace.h"

#ifdef ENDIAN_ENGINE

// writes to save file. returns 0 if ok.
// ns is namespace
// dir does not need a trailing slash
// ext is file extension, and it should not include the dot. for example, a
// json file has the extension "json", not ".json"
int save_write(const char* ns, const char* dir, const char* file, const char* ext, const char* content);

// reads from save file into out. returns 0 if ok
// ns is namespace
// dir does not need a trailing slash
// ext is file extension, and it should not include the dot. for example, a
// json file has the extension "json", not ".json"
// out buf should be set to NULL
int save_read(const char* ns, const char* dir, const char* file, const char* ext, char** out);

#endif

#define rom_load(namespace_arg, directory_arg, function_arg)                                            \
  /*                                                                                                    \
    calls 'function_arg' for every top level file or directory in                                       \
    'directory_arg' (which is relative to rom directory root) for every                                 \
    namespace 'namespace_arg' in all loaded mods                                                        \
    .                                                                                                   \
    parameters:                                                                                         \
    * namespace_arg    - const char*                                                                    \
    * directory_arg    - const char*                                                                    \
    * function_arg     - expression                                                                     \
    .                                                                                                   \
    these variables are available to be used as arguments in the function call                          \
    for 'function_arg':                                                                                 \
    * mod_name         - const char*                                                                    \
    * file_name        - const char*                                                                    \
    * file_path        - const char*                                                                    \
  */                                                                                                    \
  do {                                                                                                  \
    const char* _ns = namespace_arg;                                                                    \
    const char* _dir = directory_arg;                                                                   \
    const struct namespace* _cache = namespace_get(_ns);                                                \
                                                                                                        \
    char* _mods_path = malloc(strlen(INSTANCE_DIR) + 6 + 1);                                            \
    strcat(_mods_path, INSTANCE_DIR "/mods/");                                                          \
                                                                                                        \
    for (int _i = 0; _i < _cache->mods->length; _i++) {                                                 \
      const struct namespace_mod_entry* _modent = registry_itov(_cache->mods, _i);                      \
      char* _rom_path = malloc(strlen(_mods_path) + strlen(_modent->name) + 17 + strlen(_dir) + 1 + 1); \
      strcpy(_rom_path, _mods_path);                                                                    \
      strcat(_rom_path, _modent->name);                                                                 \
      strcat(_rom_path, "/data/endian/rom/");                                                           \
      strcat(_rom_path, _dir);                                                                          \
                                                                                                        \
      const char* mod_name = _modent->name;                                                             \
                                                                                                        \
      dir_load(_rom_path, function_arg);                                                                \
                                                                                                        \
      free(_rom_path);                                                                                  \
    }                                                                                                   \
    free(_mods_path);                                                                                   \
  } while (0)

#endif
