#ifndef ENDIAN_MIRROR_STRINGS_H_
#define ENDIAN_MIRROR_STRINGS_H_

#include <stdio.h>

#define JSMN_HEADER
#include <concord/jsmn.h>

struct mirror_strings {
  char** arr;
  size_t len;
};

void mirror_strings_append_newline_to_all(struct mirror_strings* arr);
void mirror_strings_cleanup(struct mirror_strings* arr);
int mirror_strings_from_json(struct mirror_strings* arr, const jsmntok_t* jsmn, const char* json);
// since jsmn doesnt remove escape backslashes we have to do it ourselves
// a single backslash is removed
// a double backslash is turned into one backslash
void mirror_strings_remove_backslashes(struct mirror_strings* arr);
int mirror_strings_write(FILE* file, const struct mirror_strings* arr);

#endif
