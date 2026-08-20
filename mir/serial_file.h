#ifndef ENDIAN_SERIAL_FILE_H_
#define ENDIAN_SERIAL_FILE_H_

#include <stddef.h>

#include "mirror_strings.h"

struct serial_file_tag {
  char* id;
  struct mirror_strings data;
  char* buf;
};

struct serial_file_tags {
  struct serial_file_tag* arr;
  size_t len;
};

struct serial_file {
  struct serial_file_tags tags;
};

struct serial_files {
  struct serial_file* arr;
  size_t len;
};

void serial_file_tag_cleanup(struct serial_file_tag* x);

void serial_file_tags_cleanup(struct serial_file_tags* x);

void serial_file_cleanup(struct serial_file* x);
void serial_file_load(struct serial_file* s, const char* file_path, const char* file_name);

void serial_files_cleanup(struct serial_files* x);

#endif
