#include <concord/discord.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

#include "api.h"
#include "bot.h"
#include "cli_args.h"
#include "regman.h"
#include "exit.h"

int main(int argc, const char** argv) {
  cli_args_init();
  cli_args_parse(argc, argv);

  struct sigaction sa = {0};
  sa.sa_handler = handle_sigint;
  sigaction(SIGINT, &sa, NULL);

  regman_init();
  api_init();

  bot_init();

  bot_start();

  api_call_cleanup();
  regman_cleanup();
  api_cleanup();
  cli_args_cleanup();
  bot_cleanup();

  printf("Done\n");

  return 0;
}
