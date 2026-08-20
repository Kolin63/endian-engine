#ifndef ENDIAN_CLI_ARGS_H_
#define ENDIAN_CLI_ARGS_H_

#include <stddef.h>

struct cli_args {
  bool default_root;
  char* instance_dir;
};

void cli_args_cleanup(struct cli_args* x);
void cli_args_parse(struct cli_args* x, int argc, const char** argv);

#endif
