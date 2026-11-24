#include <concord/discord.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

#include "function.h"
#include "bot.h"
#include "exit.h"
#include "regman.h"
#include "terminal.h"

int
main() {
  struct sigaction sa = {0};
  sa.sa_handler = handle_sigint;
  sigaction(SIGINT, &sa, NULL);

  regman_init();

  bot_init();

  terminal_init(sudo_terminal_get(), ENDIAN_TERMINAL_PORT);
  terminal_start(sudo_terminal_get());

  bot_start();

  terminal_cleanup(sudo_terminal_get());
  regman_cleanup();
  function_call_cleanup();
  bot_cleanup();

  printf("Done\n");

  return 0;
}
