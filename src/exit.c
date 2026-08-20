#include "exit.h"

#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#include "bot.h"
#include "log.h"
#include "function.h"
#include "regman.h"

static pthread_once_t cleanup_once = PTHREAD_ONCE_INIT;
static volatile sig_atomic_t cleanup_ready = 0;

void cleanup() {
  log_info("Stopping...");
  function_call_save();
  if (bot_get_global() != NULL) {
    discord_shutdown(bot_get_global()->discord_bot);
  }
}

void handle_exit() {
  if (cleanup_ready == 0) {
    log_warn("Cannot exit yet");
    return;
  }
  pthread_once(&cleanup_once, cleanup);
}
void handle_sigint(int) { handle_exit(); }

void set_cleanup_ready() { cleanup_ready = 1; }
int get_cleanup_ready() { return cleanup_ready; }

void abort_cleanup(int code) {
  if (regman_get() == NULL) exit(code);
  regman_cleanup();

  if (bot_get_global() == NULL) exit(code);
  if (bot_get_global()->discord_bot != NULL) discord_shutdown(bot_get_global()->discord_bot);
  bot_cleanup();

  exit(code);
}
