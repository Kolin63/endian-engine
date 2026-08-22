#include "cli_args.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

void
print_usage(const char* program_name) {
  printf("Usage: %s [-r] <instance_name>\n", program_name);
}

void
cli_args_cleanup(struct cli_args* x) {
  if (x == NULL) return;
  if (x->instance_dir != NULL) free(x->instance_dir);
}

void
cli_args_parse(struct cli_args* x, int argc, const char** argv) {
  x->default_root = true;
  x->instance_dir = NULL;

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {  // a flag
      for (size_t j = 1; j < strlen(argv[i]); j++) {
        switch (argv[i][j]) {
        case 'r':
          x->default_root = false;
          break;
        default:
          print_usage(argv[0]);
          cli_args_cleanup(x);
          exit(1);
        }
      }
    } else {  // not a flag; the instance name
      if (x->instance_dir != NULL) {
        print_usage(argv[0]);
        cli_args_cleanup(x);
        exit(1);
      }
      x->instance_dir = malloc(strlen(argv[i]) + 1);
      strcpy(x->instance_dir, argv[i]);
    }
  }  // done parsing args

  if (x->instance_dir == NULL) {
    print_usage(argv[0]);
    cli_args_cleanup(x);
    exit(1);
  }

  if (x->default_root == true) {
    char* buf = x->instance_dir;
#ifdef __linux__
    const char* home = getenv("HOME");
    if (!home) {
      log_error("Error: could not get value of $HOME");
      cli_args_cleanup(x);
      exit(1);
    }
    x->instance_dir = malloc(strlen(home) + 21 + strlen(buf) + 1);
    strcpy(x->instance_dir, home);
    strcat(x->instance_dir, "/.local/share/endian/");
#else
    static_assert(false, "Default root not supported on this OS");
#endif
    strcat(x->instance_dir, buf);
    free(buf);
  }
}
