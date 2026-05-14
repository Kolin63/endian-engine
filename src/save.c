#include "save.h"

#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>

#include "bot.h"
#include "fileio.h"
#include "log.h"
#include "sds.h"

int save_write(const char* dir, const char* file, const char* ext, const char* content) {
  sds path = sdsnew(bot_get_global()->instance_dir);
  path = sdscat(path, "/save/");
  path = sdscat(path, dir);

  // check that the directory exists
  FILE* dir_check = fopen(path, "r");
  if (dir_check == NULL) {
    mkdir(path, 0b111111111);
  } else {
    fclose(dir_check);
  }

  path = sdscat(path, "/");
  path = sdscat(path, file);
  path = sdscat(path, ".");
  path = sdscat(path, ext);

  // check that the file exists
  FILE* file_handle = fopen(path, "w");
  if (file_handle == NULL) {
    log_error("Could not open file %s for writing", path);
    return 2;
  }

  fprintf(file_handle, "%s", content);
  fclose(file_handle);
  sdsfree(path);
  return 0;
}

// predir should be "save" or "mods/modname/data/rom"
int save_or_rom_read(const char* predir, const char* dir, const char* file, const char* ext, char** out) {
  sds path = sdsnew(bot_get_global()->instance_dir);
  path = sdscat(path, "/");
  path = sdscat(path, predir);
  path = sdscat(path, "/");
  path = sdscat(path, dir);

  // check that the directory exists
  FILE* dir_check = fopen(path, "r");
  if (dir_check == NULL) {
    log_error("Directory %s does not exist", path);
    return 1;
  }
  fclose(dir_check);

  path = sdscat(path, "/");
  path = sdscat(path, file);
  path = sdscat(path, ".");
  path = sdscat(path, ext);

  // check that the file exists
  FILE* file_handle = fopen(path, "r");
  if (file_handle == NULL) {
    log_error("File %s does not exist", path);
    return 2;
  }

  assert(*out == NULL);
  *out = fileio_read_all(file_handle);
  fclose(file_handle);
  sdsfree(path);
  return 0;
}

int save_read(const char* dir, const char* file, const char* ext, char** out) {
  return save_or_rom_read("save", dir, file, ext, out);
}
