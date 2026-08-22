#ifndef ENDIAN_MOD_STACK_H_
#define ENDIAN_MOD_STACK_H_

struct mod_stack {
  const char* mod;
  const char* ns;
  const char* file;
};

struct mod_stack* mod_stack_global();

#define MOD_STACK_FMT "%s:%s:%s "
#define MOD_STACK_ARG mod_stack_global()->mod, mod_stack_global()->ns, mod_stack_global()->file

#endif
