#include "cli_args.h"
#include "mirror.h"
#include "mod_loader.h"

int main(const int argc, const char** argv) {
  struct cli_args cli_args;
  cli_args_parse(&cli_args, argc, argv);

  mod_loader_load_mods(cli_args.instance_dir);

  mirrors_cleanup(mirrors_global());
  cli_args_cleanup(&cli_args);
  return 0;
}
