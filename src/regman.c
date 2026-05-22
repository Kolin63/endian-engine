#include "regman.h"

#include <concord/discord_codecs.h>
#include <dlfcn.h>
#include <stdlib.h>

#include "command.h"
#include "function.h"
#include "namespace.h"
#include "user.h"
#include "plugin.h"
#include "registry.h"

struct regman* global;

void regman_init() {
  global = malloc(sizeof(struct regman));

  registry_init(&(global->namespace), sizeof(struct namespace), (void*)namespace_cmp, (void*)namespace_cleanup);
  registry_init(&(global->plugin), sizeof(struct plugin), (void*)plugin_cmp, (void*)plugin_cleanup);
  registry_init(&(global->function), sizeof(struct function), (void*)function_cmp, (void*)function_cleanup);
  registry_init(&(global->command), sizeof(struct command), (void*)command_cmp, (void*)command_cleanup);
  registry_init(&(global->user), sizeof(struct user*), (void*)user_cmp, (void*)user_cleanup);
}

void regman_cleanup() {
  registry_cleanup(&(global->namespace));
  registry_cleanup(&(global->plugin));
  registry_cleanup(&(global->function));
  registry_cleanup(&(global->command));
  registry_cleanup(&(global->user));
  free(global);
}

struct regman* regman_get() { return global; }

struct registry* regman_get_namespace() { return &(global->namespace); }
struct registry* regman_get_plugin() { return &(global->plugin); }
struct registry* regman_get_function() { return &(global->function); }
struct registry* regman_get_command() { return &(global->command); }
struct registry* regman_get_user() { return &(global->user); }
