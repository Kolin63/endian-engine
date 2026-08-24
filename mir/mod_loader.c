#include "mod_loader.h"

#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "core_mirrors.h"
#include "log.h"
#include "mod_stack.h"
#include "mirror.h"
#include "../src/fileio.h"
#include "serial_file.h"
#include "reflection.h"

#define mod_dir_load(pre_path, path, func)                            \
  {                                                                   \
    char* fullpath = malloc(strlen(pre_path) + 1 + strlen(path) + 1); \
    strcpy(fullpath, pre_path);                                       \
    strcat(fullpath, "/" path);                                       \
    dir_load(fullpath, func);                                         \
    free(fullpath);                                                   \
  }

void
mod_loader_mod_ns_data_dir_load(struct serial_files* files, const char* file_path, const char* file_name) {
  files->len++;
  files->arr = realloc(files->arr, files->len * sizeof(struct serial_file));
  struct serial_file* sf = files->arr + files->len - 1;
  serial_file_load(sf, file_path, file_name);

  if (sf->tags.len == 0) {
    serial_file_cleanup(sf);
    files->len--;
    files->arr = realloc(files->arr, files->len * sizeof(struct serial_file));
  }
}

void
mod_loader_mod_ns_data_load(const char* file_path, const char* file_name) {
  for (size_t i = 0; i < mirrors_global()->len; i++) {
    const struct mirror* mir = mirrors_global()->arr + i;

    if (strcmp(mir->id, file_name) == 0) {
      struct serial_files files = {};

      mod_dir_load(file_path, "", mod_stack_global()->file = file_name; mod_loader_mod_ns_data_dir_load(&files, file_path, file_name));

      struct reflection ref;
      reflection_init(&ref, mir);
      reflection_gen(&ref, &files);
      reflection_cleanup(&ref);

      serial_files_cleanup(&files);
    }
  }
}

void
mod_loader_mod_ns_load(const char* file_path) {
  mod_dir_load(file_path, "", mod_stack_global()->ns = file_name; mod_loader_mod_ns_data_load(file_path, file_name));
}

void
mod_loader_mod_load(const char* mod_path) {
  log_info("Loading mod %s", mod_stack_global()->mod);

  mod_stack_global()->file = "core_mirrors.h";
  mirror_load_from_str(ENDIAN_CORE_MIRRORS_COMMANDS);

  mod_dir_load(mod_path, "mirrors", mod_stack_global()->file = file_name; mirror_load(file_path));
  mod_stack_global()->file = "";

  mod_dir_load(mod_path, "data", mod_stack_global()->ns = file_name; mod_loader_mod_ns_load(file_path));
}

void
mod_loader_load_mods(const char* instance_dir) {
  log_info("Loading mods!");

  mod_dir_load(instance_dir, "mods", mod_stack_global()->mod = file_name; mod_loader_mod_load(file_path));
}
