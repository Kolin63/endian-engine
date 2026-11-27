#include "mod_loader.h"

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "log.h"
#include "mod_stack.h"
#include "mirror.h"
#include "../src/fileio.h"
#include "serial_file.h"
#include "reflection.h"
#include "instance_dir.h"
#include "linker.h"

#define mod_dir_load(pre_path, path, func)                            \
  {                                                                   \
    char* fullpath = malloc(strlen(pre_path) + 1 + strlen(path) + 1); \
    strcpy(fullpath, pre_path);                                       \
    strcat(fullpath, "/" path);                                       \
    if (fileio_ensure_dir_exists(fullpath) != 0) {                    \
      log_error("Could not make directory %s", fullpath);             \
      assert(false);                                                  \
    }                                                                 \
    dir_load(fullpath, func);                                         \
    free(fullpath);                                                   \
  }

// serial file linked list. we first put all of the serial files in here so that
// they are alphabetized and we have a fast write, then we move it into a
// contiguous array
struct sflink_head {
  struct sflink* arr;
};

struct sflink {
  struct serial_file sf;
  struct sflink* next;
};

static struct sflink*
sflink_add(struct sflink_head* arr, struct serial_file sf) {
  struct sflink* newlink = malloc(sizeof(struct sflink));
  newlink->sf = sf;
  newlink->next = NULL;
  struct sflink* prev = NULL;

  if (arr->arr == NULL) {
    arr->arr = newlink;
    return newlink;
  }

  for (struct sflink* i = arr->arr; i != NULL; i = i->next) {
    int ns_cmp = strcmp(i->sf.ns, newlink->sf.ns);
    if (ns_cmp < 0) {
      prev = i;
      continue;
    }

    int name_cmp = strcmp(i->sf.name, newlink->sf.name);
    if (name_cmp < 0) {
      prev = i;
      continue;
    }

    // if namespace and name are both equal, error
    if (ns_cmp == 0 && name_cmp == 0) {
      log_error(MOD_STACK_FMT "Already added serial file %s:%s",
                MOD_STACK_ARG, i->sf.ns, i->sf.name);
      free(newlink);
      return NULL;
    }

    // name_cmp > 0
    if (prev == NULL) {
      assert(i == arr->arr);
      arr->arr = newlink;
    } else {
      assert(prev->next = i);
      prev->next = newlink;
    }
    newlink->next = i;
    return newlink;
  }

  // if we get here, then we are appending to the end of the list
  assert(prev->next == NULL);
  prev->next = newlink;
  return newlink;
}

static void
sflink_to_array(struct sflink_head* src, struct serial_files* dest) {
  for (struct sflink* i = src->arr; i != NULL; i = i->next) {
    serial_files_append(dest, i->sf);
  }
}

static void
sflink_cleanup(struct sflink_head* sf) {
  struct sflink* i = sf->arr;
  struct sflink* next = NULL;

  while (i != NULL) {
    next = i->next;
    free(i);
    i = next;
  }
}

struct mirror_and_serial_files {
  struct mirror mir;
  struct sflink_head sfs;
};

static void
mirror_and_serial_files_cleanup(struct mirror_and_serial_files* x) {
  mirror_cleanup(&x->mir);
  sflink_cleanup(&x->sfs);
}

ENDVEC_STATIC(mirror_and_serial_files_arr, struct mirror_and_serial_files,
              mirror_and_serial_files_cleanup(arr->arr + i));

static void
mirror_and_serial_files_load_mirror(struct mirror_and_serial_files_arr* mir_sf_arr, const char* path) {
  struct mirror mir = {};
  mirror_load(&mir, path);
  if (mir.id == NULL) return;

  for (size_t i = 0; i < mir_sf_arr->len; i++) {
    if (strcmp(mir.id, mir_sf_arr->arr[i].mir.id) == 0) {
      log_error(MOD_STACK_FMT "Mirror %s already exists", MOD_STACK_ARG, mir.id);
      return;
    }
  }

  mirror_and_serial_files_arr_append(mir_sf_arr, (struct mirror_and_serial_files){.mir = mir});
}

static void
mirror_and_serial_files_reflect(struct mirror_and_serial_files_arr* mir_sf_arr) {
  for (size_t i = 0; i < mir_sf_arr->len; i++) {
    struct mirror_and_serial_files* it = mir_sf_arr->arr + i;

    struct serial_files sf_arr = {};
    sflink_to_array(&it->sfs, &sf_arr);

    struct reflection ref;
    reflection_init(&ref, &it->mir);
    reflection_gen(&ref, &sf_arr);
    reflection_cleanup(&ref);

    serial_files_cleanup(&sf_arr);
  }
}

static void
mod_loader_mod_ns_data_dir_load(struct sflink_head* sfs, const char* file_path, const char* file_name) {
  struct serial_file sf = {};
  int err = serial_file_load(&sf, mod_stack_global()->ns, file_path, file_name);

  if (err != 0) {
    serial_file_cleanup(&sf);
    return;
  }

  if (sflink_add(sfs, sf) == NULL) {
    serial_file_cleanup(&sf);
  }
}

static void
mod_loader_mod_ns_data_load(struct mirror_and_serial_files_arr* mir_sf_arr, const char* file_path, const char* file_name) {
  for (size_t i = 0; i < mir_sf_arr->len; i++) {
    struct mirror_and_serial_files* mir_sf = mir_sf_arr->arr + i;

    if (strcmp(mir_sf->mir.id, file_name) != 0) continue;
    mod_dir_load(file_path, "", mod_stack_global()->file = file_name; mod_loader_mod_ns_data_dir_load(&mir_sf->sfs, file_path, file_name));
  }
}

static void
mod_loader_mod_ns_load(struct mirror_and_serial_files_arr* mir_sf_arr, const char* file_path) {
  mod_dir_load(file_path, "", mod_stack_global()->file = file_name; mod_loader_mod_ns_data_load(mir_sf_arr, file_path, file_name));
}

static void
mod_loader_mod_load(struct mirror_and_serial_files_arr* mir_sf_arr, const char* mod_path) {
  log_info("Loading mod %s", mod_stack_global()->mod);

  mod_dir_load(mod_path, "mirrors",
               mod_stack_global()->file = file_name;
               mirror_and_serial_files_load_mirror(mir_sf_arr, file_path));

  mod_stack_global()->file = "";

  mod_dir_load(mod_path, "data", mod_stack_global()->ns = file_name; mod_loader_mod_ns_load(mir_sf_arr, file_path));

  mod_stack_global()->ns = "";
  mod_dir_load(mod_path, "src", linker_link(file_path, "ref/include", mod_stack_global()->mod, file_name));
}

void
mod_loader_load_mods() {
#ifndef END_REF_INSTANCE
  static_assert(false, "you must do: cmake . -DEND_INSTANCE=foo");
#endif
#ifndef END_REF_SRC_DIR
  static_assert(false, "src dir is not defined!!! it's cmake's fault, not yours");
#endif

  if (fileio_ensure_dir_exists(END_REF_SRC_DIR "/ref") != 0 ||
      fileio_ensure_dir_exists(END_REF_SRC_DIR "/ref/include") != 0 ||
      fileio_ensure_dir_exists(END_REF_SRC_DIR "/ref/include/ref") != 0 ||
      fileio_ensure_dir_exists(END_REF_SRC_DIR "/ref/include/endapi") != 0 ||
      fileio_ensure_dir_exists(END_REF_SRC_DIR "/ref/include/endapi/ref") != 0) {
    log_error("Could not make hardlink directories");
    return;
  }

  log_info("Loading mods!");

  char* instance_dir = instance_dir_expand();

  if (fileio_ensure_dir_exists(END_REF_INSTANCE) != 0) {
    log_error("Could not make instance dir");
    return;
  }

  instance_dir_write(instance_dir);

  mod_dir_load(END_REF_SRC_DIR, "src", linker_link_headers_only(file_path, "ref/include", "endapi", file_name));

  struct mirror_and_serial_files_arr mir_sf_arr = {};

  mod_stack_global()->mod = "engine";
  mod_stack_global()->file = END_REF_SRC_DIR "/mir/mirrors/functions.json";
  mirror_and_serial_files_load_mirror(&mir_sf_arr, END_REF_SRC_DIR "/mir/mirrors/functions.json");
  mod_stack_global()->file = END_REF_SRC_DIR "/mir/mirrors/commands.json";
  mirror_and_serial_files_load_mirror(&mir_sf_arr, END_REF_SRC_DIR "/mir/mirrors/commands.json");

  mod_dir_load(instance_dir, "mods", mod_stack_global()->mod = file_name; mod_loader_mod_load(&mir_sf_arr, file_path));

  mod_dir_load(END_REF_SRC_DIR, "ref", linker_link_headers_only(file_path, "ref/include", "ref", file_name));
  mod_dir_load(END_REF_SRC_DIR, "ref", linker_link_headers_only(file_path, "ref/include/endapi", "ref", file_name));

  mirror_and_serial_files_reflect(&mir_sf_arr);

  mirror_and_serial_files_arr_cleanup(&mir_sf_arr);

  free(instance_dir);
}
