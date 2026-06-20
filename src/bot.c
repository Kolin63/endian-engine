#include "bot.h"

#include <concord/discord.h>
#include <concord/logmod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "api.h"
#include "cli_args.h"
#include "command.h"
#include "exit.h"
#include "function.h"
#include "log.h"
#include "mod_loader.h"

static struct bot* global_bot = NULL;

static FILE* log_file = NULL;
static pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t on_ready_lock = PTHREAD_MUTEX_INITIALIZER;

void log_lock_func(bool lock, void* udata) {
  if (lock == true) {
    pthread_mutex_lock(&log_lock);
  } else {
    pthread_mutex_unlock(&log_lock);
  }
}

void on_ready(struct discord*, const struct discord_ready* event) {
  pthread_mutex_lock(&on_ready_lock);

  if (get_cleanup_ready() == 1) {
    log_warn("on_ready() called after first time");
    pthread_mutex_unlock(&on_ready_lock);
    return;
  }

  mod_loader_load_mods(event);
  api_call_get_api();
  api_call_init();
  api_call_load();
  set_cleanup_ready();
  log_info("Bot started!");

  pthread_mutex_unlock(&on_ready_lock);
}

void on_interaction(struct discord* client, const struct discord_interaction* event) {
  if (event->type != DISCORD_INTERACTION_APPLICATION_COMMAND)
    return;  // return if interaction isn't a slash command

  char* cmd_name = event->data->name;

  const struct command* cmd = command_get(cmd_name);

  if (cmd == NULL) {
    log_error("Could not find command %s", cmd_name);
    // TODO: send error message to user via discord
    return;
  }

  if (cmd->callback == NULL) {
    log_warn("Command %s has no callback", cmd_name);
    // TODO: send warning message to user via discord
    return;
  }

  const struct function* func = function_get(cmd->callback);
  if (func == NULL) {
    log_error("In command %s, could not find callback function %s", cmd_name, cmd->callback);
    // TODO: send warning message to user via discord
    return;
  }

  if (func->type != FT_CALLBACK) {
    log_error("In command %s, function %s:%s is not of type CALLBACK", cmd_name, func->fid.ns, func->fid.id);
    // TODO: send warning message to user via discord
    return;
  }

  func->function(client, event);
}

void bot_init() {
  if (global_bot != NULL) {
    log_error("Global Bot already initialized");
    return;
  }
  const struct cli_args* cli_args = cli_args_get_global();
  global_bot = malloc(sizeof(struct bot));
  global_bot->instance_dir = cli_args->instance_dir;

  char token[128];
  char* token_file_path = malloc(strlen(global_bot->instance_dir) + 10 + 1);
  strcpy(token_file_path, global_bot->instance_dir);
  strcat(token_file_path, "/token.txt");

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
  char* log_file_path = malloc(strlen(global_bot->instance_dir) + strlen(log_time_name) + 4 + 1);
  strcpy(log_file_path, global_bot->instance_dir);
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
  if (cli_args->verbose == 0) {
    logmod_logger_set_quiet(logmod_get_logger(discord_get_logmod(global_bot->discord_bot), "WEBSOCKETS"), 1);
    logmod_logger_set_quiet(logmod_get_logger(discord_get_logmod(global_bot->discord_bot), "WEBSOCKETS_RAW"), 1);
    logmod_logger_set_quiet(logmod_get_logger(discord_get_logmod(global_bot->discord_bot), "HTTP"), 1);
    logmod_logger_set_quiet(logmod_get_logger(discord_get_logmod(global_bot->discord_bot), "HTTP_RAW"), 1);
    logmod_logger_set_quiet(logmod_get_logger(discord_get_logmod(global_bot->discord_bot), "GATEWAY"), 1);
    logmod_logger_set_quiet(logmod_get_logger(discord_get_logmod(global_bot->discord_bot), "REQUEST"), 1);
    logmod_logger_set_quiet(logmod_get_logger(discord_get_logmod(global_bot->discord_bot), "RATELIMIT"), 1);
    logmod_logger_set_quiet(logmod_get_logger(discord_get_logmod(global_bot->discord_bot), "REFCOUNT"), 1);
  }

  discord_set_on_ready(global_bot->discord_bot, &on_ready);
  discord_set_on_interaction_create(global_bot->discord_bot, &on_interaction);
}

void bot_cleanup() {
  fclose(log_file);
  free(global_bot);
}

struct bot* bot_get_global() { return global_bot; }

void bot_start() { discord_run(global_bot->discord_bot); }
