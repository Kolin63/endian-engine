#include "namespace.h"

#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "registry.h"
#include "regman.h"

void namespace_load(const char* mod_name, const char* namespace_name) {
  struct namespace* ns = registry_ktov(regman_get_namespace(), &(struct namespace){.name = (void*)namespace_name});

  if (ns == NULL) {
    struct namespace ns_new;
    ns_new.name = malloc(strlen(namespace_name) + 1);
    strcpy(ns_new.name, namespace_name);
    ns_new.mods = malloc(sizeof(struct registry));
    registry_init(ns_new.mods, sizeof(struct namespace_mod_entry), (void*)namespace_mod_entry_cmp, (void*)namespace_mod_entry_cleanup);

    struct namespace_mod_entry modent;
    modent.name = malloc(strlen(mod_name) + 1);
    strcpy(modent.name, mod_name);

    if (registry_add(ns_new.mods, &modent) == NULL) {
      namespace_cleanup(&ns_new);
      log_error("Could not cache usage of namespace %s from mod %s", namespace_name, mod_name);
      return;
    }

    if (registry_add(regman_get_namespace(), &ns_new) == NULL) {
      namespace_cleanup(&ns_new);
      log_error("Could not cache namespace %s from mod %s", namespace_name, mod_name);
      return;
    }

    log_info("Caching new namespace %s from mod %s", namespace_name, mod_name);
  } else {
    struct namespace_mod_entry modent;
    modent.name = malloc(strlen(mod_name) + 1);
    strcpy(modent.name, mod_name);

    if (registry_add(ns->mods, &modent) == 0) {
      log_info("Caching mod's (%s) usage of pre-existing namespace %s", mod_name, namespace_name);
    }
  }
}

const struct namespace* namespace_get(const char* name) {
  return registry_ktov(regman_get_namespace(), &(struct namespace){.name = (void*)name});
}

int namespace_cmp(const struct namespace* a, const struct namespace* b) {
  return registry_strcmp(a->name, b->name);
}

void namespace_cleanup(struct namespace* elem) {
  free(elem->name);
  registry_cleanup(elem->mods);
  free(elem->mods);
}

const struct namespace_mod_entry* namespace_mod_entry_get(const struct namespace* ns, const char* name) {
  return registry_ktov(ns->mods, &(struct namespace_mod_entry){.name = (void*)name});
}

int namespace_mod_entry_cmp(const struct namespace_mod_entry* a, const struct namespace_mod_entry* b) {
  return registry_strcmp(a->name, b->name);
}

void namespace_mod_entry_cleanup(struct namespace_mod_entry* elem) {
  free(elem->name);
}
