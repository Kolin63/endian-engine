#ifndef ENDIAN_NAMESPACE_H_
#define ENDIAN_NAMESPACE_H_

struct namespace {
  const char* name;

  // list of strings of every mod that includes this namespace
  const char* mods[];
};

const static struct namespace namespaces[];
#include "../ref/namespaces.h"

#endif
