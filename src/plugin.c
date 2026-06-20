#include "plugin.h"

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "registry.h"
#include "regman.h"

void plugin_load(const char* plugin_path, const char* namespace_name, const char* mod_name, const char* plugin_name) {
  if (strcmp(plugin_name, "src") == 0) return;

  dlerror();  // clear error
  void* dl = dlopen(plugin_path, RTLD_NOW | RTLD_GLOBAL);
  if (!dl) {
    log_error("Error loading plugin %s:%s:%s: %s", mod_name, namespace_name, plugin_name, dlerror());
    return;
  }

  // without file extension
  char* clean_name = malloc(strlen(plugin_name) + 1);
  strcpy(clean_name, plugin_name);
  char* dot_finder = clean_name;
  while (*dot_finder != '\0' && *dot_finder != '.') dot_finder++;
  *dot_finder = '\0';

  struct plugin plugin;
  plugin.plugin = dl;
  plugin.fid.ns = malloc(strlen(namespace_name + 1));
  strcpy((char*)plugin.fid.ns, namespace_name);
  plugin.fid.id = malloc(strlen(clean_name) + 1);
  strcpy((char*)plugin.fid.id, clean_name);

  if (registry_add(regman_get_plugin(), &plugin) == NULL) {
    log_error("Plugin %s:%s:%s already registered", mod_name, namespace_name, plugin_name);
  }
  log_info("Loading plugin %s:%s:%s", mod_name, namespace_name, plugin_name);

  free(clean_name);
}

const struct plugin* plugin_get(const struct fid* fid) {
  return registry_ktov(regman_get_plugin(), &(struct plugin){.fid = *fid});
}

int plugin_cmp(const struct plugin* a, const struct plugin* b) {
  int namespace = registry_strcmp(a->fid.ns, b->fid.ns);
  if (namespace != 0) return namespace;
  return registry_strcmp(a->fid.id, b->fid.id);
}

void plugin_cleanup(struct plugin* elem) {
  free((char*)elem->fid.ns);
  free((char*)elem->fid.id);
  dlclose(elem->plugin);
}
