#include "mod_loader.h"

#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "core_mirrors.h"
#include "log.h"
#include "mod_stack.h"
#include "mirror.h"
#include "../src/fileio.h"

#define mod_dir_load(pre_path, path, func)                            \
  {                                                                   \
    char* fullpath = malloc(strlen(pre_path) + 1 + strlen(path) + 1); \
    strcpy(fullpath, pre_path);                                       \
    strcat(fullpath, "/" path);                                       \
    dir_load(fullpath, func);                                         \
    free(fullpath);                                                   \
  }

void mod_loader_mod_load(const char* mod_path) {
  log_info("Loading mod %s", mod_stack.mod);

  mod_stack.file = "core_mirrors.h";
  mirror_load_from_str(ENDIAN_CORE_MIRRORS_COMMANDS);

  mod_dir_load(mod_path, "mirrors", mod_stack.file = file_name; mirror_load(file_path));
}

void mod_loader_load_mods(const char* instance_dir) {
  log_info("Loading mods!");

  mod_dir_load(instance_dir, "mods", mod_stack.mod = file_name; mod_loader_mod_load(file_path));
}
