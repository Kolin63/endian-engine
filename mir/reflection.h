#ifndef ENDIAN_REFLECTION_H_
#define ENDIAN_REFLECTION_H_

#include <stddef.h>
#include <stdio.h>

#include "alpha_switch.h"
#include "mirror.h"
#include "mirror_strings.h"
#include "serial_file.h"
#include "../src/endvec.h"

struct reflection_out {
  const struct mirror_file* mir_file;
  struct mirror_strings buf;
  struct alpha_switch alpha;
};

ENDVEC_DECLARE(reflection_outs, struct reflection_out);

struct reflection {
  struct reflection_outs files;
};

void reflection_cleanup(struct reflection* ref);
int reflection_gen(struct reflection* ref, const struct serial_files* sf);
void reflection_init(struct reflection* ref, const struct mirror* mir);

#endif
