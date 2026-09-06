#ifndef ENDIAN_ALPHA_SWITCH_H_
#define ENDIAN_ALPHA_SWITCH_H_

#include <stddef.h>

#include "mirror_strings.h"
#include "../src/endvec.h"

ENDVEC_DECLARE(alpha_switch_matches, size_t);

struct alpha_switch_case {
  char* val;
  struct mirror_strings buf;
};

ENDVEC_DECLARE(alpha_switch_cases, struct alpha_switch_case);

struct alpha_switch {
  int escapes_encountered;
  char* switch_on;
  struct alpha_switch_cases cases;
  struct mirror_strings def;
  char* const* buf;
};

void alpha_switch_matches_append(struct alpha_switch_matches* arr, size_t x);
void alpha_switch_matches_cleanup(struct alpha_switch_matches* arr);

void alpha_switch_case_cleanup(struct alpha_switch_case* alpha);

void alpha_switch_cases_cleanup(struct alpha_switch_cases* alpha);

void alpha_switch_cleanup(struct alpha_switch* alpha);
void alpha_switch_fillout(struct alpha_switch* alpha, const char* buf);
int alpha_switch_gen(struct mirror_strings* buf, struct alpha_switch* alpha);

#endif
