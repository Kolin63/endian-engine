#ifndef ENDIAN_REGMAN_H_
#define ENDIAN_REGMAN_H_

#include "registry.h"

struct regman {
  struct registry namespace;
  struct registry plugin;
  struct registry function;
  struct registry command;
  struct registry user;
};

void regman_init();
void regman_cleanup();

struct regman* regman_get();

struct registry* regman_get_namespace();
struct registry* regman_get_plugin();
struct registry* regman_get_function();
struct registry* regman_get_command();
struct registry* regman_get_user();

#endif
