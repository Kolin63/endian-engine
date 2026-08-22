#include <concord/discord.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

#include "function.h"
#include "bot.h"
#include "exit.h"
#include "regman.h"

int
main(int argc, const char** argv) {
  struct sigaction sa = {0};
  sa.sa_handler = handle_sigint;
  sigaction(SIGINT, &sa, NULL);

  regman_init();

  bot_init();

  bot_start();

  regman_cleanup();
  function_call_cleanup();
  bot_cleanup();

  printf("Done\n");

  return 0;
}
