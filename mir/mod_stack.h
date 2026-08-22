#ifndef ENDIAN_MOD_STACK_H_
#define ENDIAN_MOD_STACK_H_

struct mod_stack {
  const char* mod;
  const char* ns;
  const char* file;
};

struct mod_stack* g_mod_stack();

#define MOD_STACK_FMT "%s:%s:%s "
#define MOD_STACK_ARG g_mod_stack()->mod, g_mod_stack()->ns, g_mod_stack()->file

#endif
