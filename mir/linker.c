#include "linker.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include <log.h>

#include "../src/fileio.h"

int
linker_link(const char* old, const char* relative_dir, const char* new_dir, const char* new_name) {
  char* path = malloc(strlen(END_REF_SRC_DIR "/") + strlen(relative_dir) + 1 + strlen(new_dir) + 1 + strlen(new_name) + 1);
  strcpy(path, END_REF_SRC_DIR "/");
  strcat(path, relative_dir);
  strcat(path, "/");
  strcat(path, new_dir);

  if (fileio_ensure_dir_exists(path) != 0) {
    log_error("Could not make hard link dir %s", path);
    free(path);
    return 1;
  }

  strcat(path, "/");
  strcat(path, new_name);

  // if the hard link already exists, we remove it
  FILE* file = fopen(path, "r");
  if (file != NULL) {
    remove(path);
    fclose(file);
  }

  if (link(old, path) != 0) {
    log_error("Could not make hardlink from %s to %s", old, path);
    return 1;
  }

  log_info("Making hardlink to %s/%s/%s", relative_dir, new_dir, new_name);

  free(path);
  return 0;
}

int
linker_link_headers_only(const char* old, const char* relative_dir, const char* new_dir, const char* new_name) {
  const size_t len = strlen(old);
  if (len < 3) return 0;

  if (old[len - 1] == 'h' && old[len - 2] == '.') {
    return linker_link(old, relative_dir, new_dir, new_name);
  } else {
    return 0;
  }
}
