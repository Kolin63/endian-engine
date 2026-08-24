#ifndef ENDIAN_REFLECTION_H_
#define ENDIAN_REFLECTION_H_

#include <stddef.h>
#include <stdio.h>

#include "mirror.h"
#include "mirror_strings.h"
#include "serial_file.h"

struct reflection_group {
  const struct mirror_group* mir;
  struct mirror_strings foreach_buf;
};

struct reflection_groups {
  struct reflection_group* arr;
  size_t len;
};

struct reflection_file {
  const char* name;
  struct reflection_groups groups;
};

struct reflection_files {
  struct reflection_file* arr;
  size_t len;
};

struct reflection {
  struct reflection_files files;
};

void reflection_group_cleanup(struct reflection_group* ref);
int reflection_group_gen(struct reflection_group* ref, const struct serial_file* sf);
int reflection_group_write(const struct reflection_group* ref);

void reflection_groups_cleanup(struct reflection_groups* ref);
int reflection_groups_gen(struct reflection_groups* ref, const struct serial_file* sf);
int reflection_groups_write(const struct reflection_groups* ref);

void reflection_file_cleanup(struct reflection_file* ref);
int reflection_file_gen(struct reflection_file* ref, const struct serial_file* sf);
int reflection_file_write(const struct reflection_file* ref);

void reflection_files_cleanup(struct reflection_files* ref);
int reflection_files_gen(struct reflection_files* ref, const struct serial_files* sf);
int reflection_files_write(const struct reflection_files* ref);

void reflection_cleanup(struct reflection* ref);
int reflection_gen(struct reflection* ref, const struct serial_files* sf);
void reflection_init(struct reflection* ref, const struct mirror* mir);

#endif
