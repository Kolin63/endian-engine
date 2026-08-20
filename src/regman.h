#ifndef ENDIAN_REGMAN_H_
#define ENDIAN_REGMAN_H_

#include "registry.h"

struct regman {
  struct registry user;
};

void regman_init();
void regman_cleanup();

struct regman* regman_get();

struct registry* regman_get_user();

#endif
