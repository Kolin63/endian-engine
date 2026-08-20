#ifndef ENDIAN_MIRROR_STRINGS_H_
#define ENDIAN_MIRROR_STRINGS_H_

#define JSMN_HEADER
#include <concord/jsmn.h>

struct mirror_strings {
  char** arr;
  size_t len;
};

void mirror_strings_cleanup(struct mirror_strings* arr);
int mirror_strings_from_json(struct mirror_strings* arr, const jsmntok_t* jsmn, const char* json);

#endif
