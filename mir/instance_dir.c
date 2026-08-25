#include "instance_dir.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <log.h>

int
instance_dir_write(const char* instance_dir) {
  FILE* file = fopen(END_REF_SRC_DIR "/ref/instance_dir.h", "w");

  if (file == NULL) {
    log_error("Could not open file ref/instance_dir.h");
    return 1;
  }

  const char* prefix =
      "\
#ifndef ENDIAN_REFLECTIONS_INSTANCE_DIR_H_\n\
#define ENDIAN_REFLECTIONS_INSTANCE_DIR_H_\n\
\n\
#define INSTANCE_DIR \"";

  const char* postfix =
      "\
\"\n\
\n\
#endif";

  if (fputs(prefix, file) == EOF) {
    log_error("Could not write to file ref/instance_dir.h");
    fclose(file);
    return 1;
  }
  if (fputs(instance_dir, file) == EOF) {
    log_error("Could not write to file ref/instance_dir.h");
    fclose(file);
    return 1;
  }
  if (fputs(postfix, file) == EOF) {
    log_error("Could not write to file ref/instance_dir.h");
    fclose(file);
    return 1;
  }

  fclose(file);
  return 0;
}

char*
instance_dir_expand() {
  char* instance_dir = strdup(END_REF_INSTANCE);
  bool default_root = !(instance_dir[0] == '/');

  if (default_root == true) {
    char* buf = instance_dir;
#ifdef __linux__
    const char* home = getenv("HOME");
    if (!home) {
      log_error("Could not get value of $HOME");
      free(instance_dir);
      exit(1);
    }
    instance_dir = malloc(strlen(home) + 21 + strlen(buf) + 1);
    strcpy(instance_dir, home);
    strcat(instance_dir, "/.local/share/endian/");
#else
    static_assert(false, "Default root not supported on this OS");
#endif
    strcat(instance_dir, buf);
    free(buf);
  }

  return instance_dir;
}
