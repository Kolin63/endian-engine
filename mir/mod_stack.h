#ifndef ENDIAN_MOD_STACK_H_
#define ENDIAN_MOD_STACK_H_

struct mod_stack {
  const char* mod;
  const char* ns;
  const char* file;
};

static struct mod_stack mod_stack = {
    .mod = "",
    .ns = "",
    .file = "",
};

#define MOD_STACK_FMT "%s:%s:%s "
#define MOD_STACK_ARG mod_stack.mod, mod_stack.ns, mod_stack.file

#endif
