#include "save.h"

#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "bot.h"
#include "fileio.h"
#include "log.h"

int save_write(const char* ns, const char* dir, const char* file,
               const char* ext, const char* content) {
  char* path = malloc(strlen(bot_get_global()->instance_dir) + 6 + strlen(ns) +
                      1 + strlen(dir) + 1 + strlen(file) + 1 + strlen(ext) + 1);
  strcpy(path, bot_get_global()->instance_dir);
  strcat(path, "/save/");
  strcat(path, ns);

  // check that the namespace directory exists
  FILE* dir_check = fopen(path, "r");
  if (dir_check == NULL) {
    mkdir(path, 0b111111111);
  } else {
    fclose(dir_check);
  }

  strcat(path, "/");
  strcat(path, dir);

  // check that the namespace/dir directory exists
  dir_check = fopen(path, "r");
  if (dir_check == NULL) {
    mkdir(path, 0b111111111);
  } else {
    fclose(dir_check);
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

// predir should be "save" or "mods/modname/data/rom"
int save_or_rom_read(const char* predir, const char* ns, const char* dir,
                     const char* file, const char* ext, char** out) {
  char* path = malloc(strlen(bot_get_global()->instance_dir) + 1 +
                      strlen(predir) + 1 + strlen(ns) + 1 + strlen(dir) + 1 +
                      strlen(file) + 1 + strlen(ext) + 1);
  strcpy(path, bot_get_global()->instance_dir);
  strcat(path, "/");
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

int save_read(const char* ns, const char* dir, const char* file,
              const char* ext, char** out) {
  return save_or_rom_read("save", ns, dir, file, ext, out);
}
