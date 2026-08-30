#ifndef ENDIAN_SERIAL_FILE_H_
#define ENDIAN_SERIAL_FILE_H_

#include <stdio.h>
#include <stddef.h>

#include "mirror_strings.h"
#include "../src/endvec.h"

struct serial_file_tag {
  char* id;
  char* data;
  char* buf;
};

ENDVEC_DECLARE(serial_file_tags, struct serial_file_tag);

struct serial_file {
  char* name;
  struct serial_file_tags tags;
};

ENDVEC_DECLARE(serial_files, struct serial_file);

void serial_file_tag_cleanup(struct serial_file_tag* x);

void serial_file_tags_cleanup(struct serial_file_tags* x);
int serial_file_tags_fillout(struct serial_file_tags* tags, FILE* file);

void serial_file_cleanup(struct serial_file* x);
void serial_file_load(struct serial_file* s, const char* file_path, const char* file_name);

void serial_files_cleanup(struct serial_files* x);

#endif
