#ifndef ENDIAN_MIRROR_H_
#define ENDIAN_MIRROR_H_

#include <stddef.h>

#define JSMN_HEADER
#include <concord/jsmn.h>

#include "mirror_strings.h"

struct mirror_foreach {
  char* tag;
  struct mirror_strings format;
};

struct mirror_foreach_arr {
  struct mirror_foreach* arr;
  size_t len;
};

struct mirror_group {
  struct mirror_strings prefix;
  struct mirror_foreach_arr foreach;
  struct mirror_strings postfix;
};

struct mirror_groups {
  struct mirror_group* arr;
  size_t len;
};

struct mirror_file {
  char* name;
  struct mirror_groups groups;
};

struct mirror_files {
  struct mirror_file* arr;
  size_t len;
};

struct mirror {
  char* id;
  struct mirror_files files;
};

struct mirrors {
  struct mirror* arr;
  size_t len;
};

static struct mirrors mirrors = {};

void mirror_foreach_cleanup(struct mirror_foreach* f);
int mirror_foreach_from_json(struct mirror_foreach* f, const jsmntok_t* jsmn, const char* json);

void mirror_foreach_arr_cleanup(struct mirror_foreach_arr* arr);
int mirror_foreach_arr_from_json(struct mirror_foreach_arr* arr, const jsmntok_t* jsmn, const char* json);

void mirror_group_cleanup(struct mirror_group* g);
int mirror_group_from_json(struct mirror_group* g, const jsmntok_t* jsmn, const char* json);

void mirror_groups_cleanup(struct mirror_groups* arr);
int mirror_groups_from_json(struct mirror_groups* arr, const jsmntok_t* jsmn, const char* json);

void mirror_file_cleanup(struct mirror_file* f);
int mirror_file_from_json(struct mirror_file* f, const jsmntok_t* jsmn, const char* json);

void mirror_files_cleanup(struct mirror_files* arr);
int mirror_files_from_json(struct mirror_files* arr, const jsmntok_t* jsmn, const char* json);

void mirror_cleanup(struct mirror* m);
int mirror_from_json(struct mirror* m, const jsmntok_t* jsmn, const char* json);
void mirror_load(const char* file_path);
void mirror_load_from_str(const char* str);

void mirrors_cleanup(struct mirrors* arr);

#endif
