#include "api.h"

#include <concord/interaction.h>
#include <stdlib.h>

#include "exit.h"
#include "function.h"
#include "log.h"
#include "registry.h"
#include "regman.h"
#include "save.h"
#include "user.h"
#include "command.h"
#include "plugin.h"
#include "namespace.h"
#include "fileio.h"

static struct api* global;

void api_init() {
  global = malloc(sizeof(struct api));

  global->version = API_VERSION;

  global->log_log = log_log;

  global->handle_exit = handle_exit;
  global->handle_save = api_call_save;
  global->abort_cleanup = abort_cleanup;
  global->get_cleanup_ready = get_cleanup_ready;

  global->discord_create_interaction_response = discord_create_interaction_response;

  global->registry_init = registry_init;
  global->registry_cleanup = registry_cleanup;
  global->registry_add = registry_add;
  global->registry_del_val = registry_del_val;
  global->registry_del_key = registry_del_key;
  global->registry_del_i = registry_del_i;
  global->registry_itov = registry_itov;
  global->registry_itov_safe = registry_itov_safe;
  global->registry_ktoi = registry_ktoi;
  global->registry_ktov = registry_ktov;
  global->registry_vtoi = registry_vtoi;
  global->registry_strcmp = registry_strcmp;

  global->get_namespace_registry = (void*)regman_get_namespace;
  global->namespace_get = namespace_get;
  global->namespace_mod_entry_get = namespace_mod_entry_get;
  global->get_plugin_registry = (void*)regman_get_plugin;
  global->plugin_get = plugin_get;
  global->get_function_registry = (void*)regman_get_function;
  global->function_get = function_get;
  global->get_command_registry = (void*)regman_get_command;
  global->command_get = command_get;

  global->jsmn_iterator_get_string = jsmn_iterator_get_string;
  global->jsmn_iterator_get_string_heap = jsmn_iterator_get_string_heap;
  global->jsmn_iterator_type_to_str = jsmn_iterator_type_to_str;
  global->jsmn_iterator_init = jsmn_iterator_init;
  global->jsmn_iterator_next = jsmn_iterator_next;

  global->fileio_read_all = fileio_read_all;
  global->fileio_read_json = fileio_read_json;

  global->bot_get_global = bot_get_global;

  global->user_init = user_init;
  global->user_get = user_get;
  global->user_cmp = user_cmp;
  global->user_cleanup = user_cleanup;
  global->uuid_to_string = uuid_to_string;
  global->string_to_uuid = string_to_uuid;

  global->save_write = save_write;
  global->save_read = save_read;
}

void api_cleanup() { free(global); }

const struct api* api_get_global() { return global; }

void api_call_func_type(enum function_type type) {
  const struct registry* func_reg = regman_get_function();
  for (int i = 0; i < func_reg->length; i++) {
    const struct function* func = registry_itov(func_reg, i);
    if (func->type != type) continue;
    if (type == FT_GET_API) func->function(global);
    else func->function();
  }
}

void api_call_get_api() { api_call_func_type(FT_GET_API); }

void api_call_init() { api_call_func_type(FT_INIT); }

void api_call_load() { api_call_func_type(FT_LOAD); }

void api_call_save() { api_call_func_type(FT_SAVE); }

void api_call_cleanup() { api_call_func_type(FT_CLEANUP); }
