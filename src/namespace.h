#ifndef ENDIAN_NAMESPACE_H_
#define ENDIAN_NAMESPACE_H_

#include "registry.h"

struct namespace_mod_entry {
  char* name;
};

struct namespace {
  char* name;

  // registry of strings of every mod that includes this namespace
  struct registry* mods;
};

void namespace_load(const char* mod_name, const char* namespace_name);
const struct namespace* namespace_get(const char* name);

int namespace_cmp(const struct namespace* a, const struct namespace* b);
void namespace_cleanup(struct namespace* elem);

const struct namespace_mod_entry* namespace_mod_entry_get(const struct namespace* ns, const char* name);

int namespace_mod_entry_cmp(const struct namespace_mod_entry* a, const struct namespace_mod_entry* b);
void namespace_mod_entry_cleanup(struct namespace_mod_entry* elem);

#endif
