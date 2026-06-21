#include "mod_loader.h"

#include <concord/discord.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "bot.h"
#include "command.h"
#include "function.h"
#include "log.h"
#include "namespace.h"
#include "plugin.h"
#include "save.h"

#define mod_dir_load(pre_path, path, func)                            \
  {                                                                   \
    char* fullpath = malloc(strlen(pre_path) + 1 + strlen(path) + 1); \
    strcpy(fullpath, pre_path);                                       \
    strcat(fullpath, "/" path);                                       \
    dir_load(fullpath, func);                                         \
    free(fullpath);                                                   \
  }

void mod_loader_namespace_load(const struct discord_ready* event, const char* namespace_path, const char* namespace_name, const char* mod_name) {
  namespace_load(mod_name, namespace_name);
  mod_dir_load(namespace_path, "plugins", plugin_load(file_path, mod_name, namespace_name, file_name));
  mod_dir_load(namespace_path, "functions", function_load(file_path, mod_name, namespace_name, file_name));
  mod_dir_load(namespace_path, "commands", command_load(event, file_path, mod_name, file_name));
}

void mod_loader_mod_load(const struct discord_ready* event, const char* mod_path, const char* mod_name) {
  log_info("Loading mod %s", mod_name);

  mod_dir_load(mod_path, "data", mod_loader_namespace_load(event, file_path, file_name, mod_name));
}

void mod_loader_load_mods(const struct discord_ready* event) {
  log_info("Loading mods!");

  // this is for logging within the macro
  const char* mod_name = "ENDIAN_ENGINE";

  mod_dir_load(bot_get_global()->instance_dir, "mods", mod_loader_mod_load(event, file_path, file_name));
}
