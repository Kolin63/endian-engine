#include "bot.h"

#include <concord/discord.h>
#include <concord/logmod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "command.h"
#include "exit.h"
#include "log.h"
#include "function.h"
#include "endapi/ref/instance_dir.h"

static struct bot* global_bot = NULL;

static FILE* log_file = NULL;
static pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t on_ready_lock = PTHREAD_MUTEX_INITIALIZER;

void
log_lock_func(bool lock, void* udata) {
  if (lock == true) {
    pthread_mutex_lock(&log_lock);
  } else {
    pthread_mutex_unlock(&log_lock);
  }
}

void
on_ready(struct discord*, const struct discord_ready* event) {
  pthread_mutex_lock(&on_ready_lock);

  if (get_cleanup_ready() == 1) {
    log_warn("on_ready() called after first time");
    pthread_mutex_unlock(&on_ready_lock);
    return;
  }

  function_call_init();
  function_call_load();
  set_cleanup_ready();
  log_info("Bot started!");

  pthread_mutex_unlock(&on_ready_lock);
}

void
on_interaction(struct discord* client, const struct discord_interaction* event) {
  if (event->type != DISCORD_INTERACTION_APPLICATION_COMMAND)
    return;  // return if interaction isn't a slash command

  const char* cmd_name = event->data->name;

  enum command_id cmd_id = command_id_get(cmd_name);

  cmd_id = COMMAND_PING;

  if (cmd_id == COMMAND_UNKNOWN_) {
    log_error("Could not find command %s", cmd_name);
    // TODO: send error message to user via discord
    return;
  }

  command_cb(cmd_id, client, event);
}

void
bot_init() {
  if (global_bot != NULL) {
    log_error("Global Bot already initialized");
    return;
  }
  global_bot = malloc(sizeof(struct bot));

  char token[128];
  char* token_file_path = malloc(strlen(INSTANCE_DIR) + 10 + 1);
  strcat(token_file_path, INSTANCE_DIR "/token.txt");

  FILE* token_file = fopen(token_file_path, "r");

  if (!token_file) {
    log_error("Could not open token file: %s", token_file_path);
    abort_cleanup(EXIT_FAILURE);
  }

  fgets(token, sizeof(token), token_file);
  fclose(token_file);

  if (strlen(token) == 0) {
    log_error("Bot token is of length 0");
    abort_cleanup(EXIT_FAILURE);
  }

  // a new line in the token breaks concord
  if (token[strlen(token) - 1] == '\n') {
    token[strlen(token) - 1] = '\0';
  }

  global_bot->discord_bot = discord_init(token);

  free(token_file_path);

  // open logging files
  char log_time_name[128];

  time_t rawtime;
  struct tm* tm_time;
  time(&rawtime);
  tm_time = localtime(&rawtime);
  strftime(log_time_name, sizeof(log_time_name), "/logs/%Y-%m-%d-%H:%M:%S", tm_time);

  // setup logging from endian
  char* log_file_path = malloc(strlen(INSTANCE_DIR) + strlen(log_time_name) + 4 + 1);
  strcpy(log_file_path, INSTANCE_DIR);
  strcat(log_file_path, log_time_name);
  strcat(log_file_path, ".log");

  log_file = fopen(log_file_path, "w");

  if (!log_file) {
    log_error("Could not open file for Endian logging at %s", log_file_path);
    free(log_file_path);
    abort_cleanup(EXIT_FAILURE);
  }

  log_add_fp(log_file, LOG_TRACE);
  log_set_lock(log_lock_func, NULL);

  free(log_file_path);

  // setup logging from concord
  logmod_logger_set_quiet(logmod_get_logger(discord_get_logmod(global_bot->discord_bot), "WEBSOCKETS"), 1);
  logmod_logger_set_quiet(logmod_get_logger(discord_get_logmod(global_bot->discord_bot), "WEBSOCKETS_RAW"), 1);
  logmod_logger_set_quiet(logmod_get_logger(discord_get_logmod(global_bot->discord_bot), "HTTP"), 1);
  logmod_logger_set_quiet(logmod_get_logger(discord_get_logmod(global_bot->discord_bot), "HTTP_RAW"), 1);
  logmod_logger_set_quiet(logmod_get_logger(discord_get_logmod(global_bot->discord_bot), "GATEWAY"), 1);
  logmod_logger_set_quiet(logmod_get_logger(discord_get_logmod(global_bot->discord_bot), "REQUEST"), 1);
  logmod_logger_set_quiet(logmod_get_logger(discord_get_logmod(global_bot->discord_bot), "RATELIMIT"), 1);
  logmod_logger_set_quiet(logmod_get_logger(discord_get_logmod(global_bot->discord_bot), "REFCOUNT"), 1);

  discord_set_on_ready(global_bot->discord_bot, &on_ready);
  discord_set_on_interaction_create(global_bot->discord_bot, &on_interaction);
}

void
bot_cleanup() {
  fclose(log_file);
  free(global_bot);
}

struct bot*
bot_get_global() { return global_bot; }

void
bot_start() { discord_run(global_bot->discord_bot); }
