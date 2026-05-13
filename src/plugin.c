#include "plugin.h"

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "registry.h"
#include "regman.h"
#include "sds.h"

void plugin_load(const char* plugin_path, const char* namespace_name, const char* mod_name, const char* plugin_name) {
  if (strcmp(plugin_name, "src") == 0) return;

  dlerror();  // clear error
  void* dl = dlopen(plugin_path, RTLD_NOW | RTLD_GLOBAL);
  if (!dl) {
    log_error("Error loading plugin %s:%s:%s: %s", mod_name, namespace_name, plugin_name, dlerror());
    return;
  }

  // without file extension
  sds clean_name = sdsnew(plugin_name);
  char* dot_finder = clean_name;
  while (*dot_finder != '\0' && *dot_finder != '.') dot_finder++;
  sdsrange(clean_name, 0, dot_finder - clean_name - 1);

  struct plugin plugin;
  plugin.plugin = dl;
  plugin.namespace = malloc(strlen(namespace_name + 1));
  strcpy(plugin.namespace, namespace_name);
  plugin.name = malloc(strlen(clean_name) + 1);
  strcpy(plugin.name, clean_name);

  if (registry_add(regman_get_plugin(), &plugin) == NULL) {
    log_error("Plugin %s:%s:%s already registered", mod_name, namespace_name, plugin_name);
  }
  log_info("Loading plugin %s:%s:%s", mod_name, namespace_name, plugin_name);

  sdsfree(clean_name);
}

const struct plugin* plugin_get(char* namespace, char* name) {
  return registry_ktov(regman_get_plugin(), &(struct plugin){.namespace = namespace, .name = name});
}

int plugin_cmp(const struct plugin* a, const struct plugin* b) {
  int namespace = registry_strcmp(a->namespace, b->namespace);
  if (namespace != 0) return namespace;
  return registry_strcmp(a->name, b->name);
}

void plugin_cleanup(struct plugin* elem) {
  free(elem->namespace);
  free(elem->name);
  dlclose(elem->plugin);
}
