#include "save.h"

#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "bot.h"
#include "fileio.h"
#include "log.h"
#include "endapi/ref/instance_dir.h"

int
save_write(const char* ns, const char* dir, const char* file,
           const char* ext, const char* content) {
  char* path = malloc(strlen(INSTANCE_DIR) + 6 + strlen(ns) +
                      1 + strlen(dir) + 1 + strlen(file) + 1 + strlen(ext) + 1);
  strcat(path, INSTANCE_DIR "/saves/");
  strcat(path, ns);

  if (fileio_ensure_dir_exists(path) != 0) {
    log_error("Could not write save file as a result of failure to make dir %s", path);
  }

  strcat(path, "/");
  strcat(path, dir);

  // check that the namespace/dir directory exists
  if (fileio_ensure_dir_exists(path) != 0) {
    log_error("Could not write save file as a result of failure to make dir %s", path);
  }

  strcat(path, "/");
  strcat(path, file);
  strcat(path, ".");
  strcat(path, ext);

  // check that the file exists
  FILE* file_handle = fopen(path, "w");
  if (file_handle == NULL) {
    log_error("Could not open file %s for writing. Tried to write: %s", path, content);
    return 2;
  }

  fprintf(file_handle, "%s", content);
  fclose(file_handle);
  free(path);
  return 0;
}

// predir should be "saves" or "mods/modname/data/rom"
int
save_or_rom_read(const char* predir, const char* ns, const char* dir,
                 const char* file, const char* ext, char** out) {
  char* path = malloc(strlen(INSTANCE_DIR) + 1 +
                      strlen(predir) + 1 + strlen(ns) + 1 + strlen(dir) + 1 +
                      strlen(file) + 1 + strlen(ext) + 1);
  strcat(path, INSTANCE_DIR "/");
  strcat(path, predir);
  strcat(path, "/");
  strcat(path, ns);

  // check that the namespace directory exists
  FILE* dir_check = fopen(path, "r");
  if (dir_check == NULL) {
    log_error("Directory %s does not exist", path);
    return 1;
  }
  fclose(dir_check);

  strcat(path, "/");
  strcat(path, dir);

  // check that the namespace/dir directory exists
  fopen(path, "r");
  if (dir_check == NULL) {
    log_error("Directory %s does not exist", path);
    return 1;
  }
  fclose(dir_check);

  strcat(path, "/");
  strcat(path, file);
  strcat(path, ".");
  strcat(path, ext);

  // check that the file exists
  FILE* file_handle = fopen(path, "r");
  if (file_handle == NULL) {
    log_error("File %s does not exist", path);
    return 2;
  }

  assert(*out == NULL);
  *out = fileio_read_all(file_handle);
  fclose(file_handle);
  free(path);
  return 0;
}

int
save_read(const char* ns, const char* dir, const char* file,
          const char* ext, char** out) {
  return save_or_rom_read("saves", ns, dir, file, ext, out);
}
