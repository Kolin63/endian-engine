#ifndef ENDIAN_MIRROR_H_
#define ENDIAN_MIRROR_H_

#include <stddef.h>

#define JSMN_HEADER
#include <concord/jsmn.h>

#include "mirror_strings.h"
#include "../src/endvec.h"

enum mirror_format_token_type {
  MIR_CONST,
  MIR_FOREACH_BEGIN,  // the tag for the block is stored in buf
  MIR_FOREACH_END,
  MIR_TAG_CONTENT,   // %t
  MIR_DATA,          // %d
  MIR_DATA_CAPS,     // %D
  MIR_MOD,           // %m
  MIR_MOD_CAPS,      // %M
  MIR_NS,            // %n
  MIR_NS_CAPS,       // %N
  MIR_ALPHA_SWITCH,  // %A
  MIR_BLAME,         // %B
};

struct mirror_format_token {
  enum mirror_format_token_type type;
  char* buf;
};

ENDVEC_DECLARE(mirror_format_tokens, struct mirror_format_token);

struct mirror_file {
  char* name;
  struct mirror_format_tokens tokens;
};

ENDVEC_DECLARE(mirror_files, struct mirror_file);

struct mirror {
  char* id;
  struct mirror_files files;
};

ENDVEC_DECLARE(mirrors, struct mirror);

void mirror_cleanup(struct mirror* m);
int mirror_from_json(struct mirror* m, const jsmntok_t* jsmn, const char* json);
void mirror_load(const char* file_path);
void mirror_load_from_str(const char* str);

void mirrors_cleanup(struct mirrors* arr);
struct mirrors* mirrors_global();

#endif
