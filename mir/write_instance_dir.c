#include "write_instance_dir.h"

#include <stdio.h>

#include <log.h>

int
write_instance_dir(const char* instance_dir) {
  FILE* file = fopen("ref/instance_dir.h", "w");

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
