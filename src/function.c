#include "function.h"

#include <concord/jsmn.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fid.h"
#include "fileio.h"
#include "jsmn_iterator.h"
#include "json_macros.h"
#include "log.h"
#include "plugin.h"
#include "registry.h"
#include "regman.h"

// returns amount of errors, 0 if ok
int function_fillout(const char* namespace_name, const char* mod_name,
                     const char* file_name, const jsmntok_t* jsmn,
                     const char* json, struct function* func) {
  int error = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_OBJECT_AND_CHILDREN_RET(iter, error++; return error);

  while (jsmn_iterator_next(&iter)) {
    if (strcmp(iter.key, "type") == 0) {
      END_JSON_CHECK_STRING(iter);
      char val[128];
      jsmn_iterator_get_string(val, 128, json, iter.val);
      if (strcmp(val, "CALLBACK") == 0) func->type = FT_CALLBACK;
      else if (strcmp(val, "GET_API") == 0) func->type = FT_GET_API;
      else if (strcmp(val, "INIT") == 0) func->type = FT_INIT;
      else if (strcmp(val, "LOAD") == 0) func->type = FT_LOAD;
      else if (strcmp(val, "FILLOUT") == 0) func->type = FT_FILLOUT;
      else if (strcmp(val, "EXPORT") == 0) func->type = FT_EXPORT;
      else if (strcmp(val, "TO_JSON") == 0) func->type = FT_TO_JSON;
      else if (strcmp(val, "DATA") == 0) func->type = FT_DATA;
      else if (strcmp(val, "SAVE") == 0) func->type = FT_SAVE;
      else if (strcmp(val, "CLEANUP") == 0) func->type = FT_CLEANUP;
      else {
        log_error("In function %s:%s:%s, unknown type %s", namespace_name, mod_name, file_name, val);
        error++;
        continue;
      }
    } else if (strcmp(iter.key, "name") == 0) {
      END_JSON_CHECK_STRING(iter);
      func->name = jsmn_iterator_get_string_heap(json, iter.val);
    } else if (strcmp(iter.key, "plugin") == 0) {
      END_JSON_CHECK_STRING(iter);
      // namespace:plugin
      char* str = jsmn_iterator_get_string_heap(json, iter.val);

      struct fid fid = fid_split(str);

      if (fid.ns == NULL) {
        log_error("Could not load plugin name in format namespace:plugin from %s:%s:%s", mod_name, namespace_name, file_name);
        free(str);
        error++;
        continue;
      }

      const struct plugin* plugin = plugin_get(&fid);
      func->plugin_fid = &plugin->fid;

      free(str);
    } else {
      log_error("Function %s:%s:%s has unknown object %s", namespace_name, mod_name, file_name, iter.key);
      error++;
      continue;
    }
  }

  return error;
}

void function_load(const char* function_path, const char* namespace_name, const char* mod_name, const char* file_name) {
  if (strcmp(file_name, "template.json") == 0) return;

  FILE* file = fopen(function_path, "r");
  if (file == NULL) {
    log_error("Could not open %s at %s from %s:%s", file_name, function_path, namespace_name, mod_name);
    return;
  }
  char* json = fileio_read_all(file);
  fclose(file);

  jsmntok_t* jsmn = fileio_read_json(json);

  struct function func = {};
  if (function_fillout(namespace_name, mod_name, file_name, jsmn, json, &func) != 0) {
    free(json);
    free(jsmn);
    return;
  }

  free(json);
  free(jsmn);

  const struct plugin* plugin = plugin_get(func.plugin_fid);
  if (plugin == NULL) {
    log_error("Could not find plugin %s:%s while loading function %s:%s:%s",
              func.plugin_fid->ns, func.plugin_fid->id, namespace_name, mod_name, func.name);
    return;
  }

  dlerror();  // clear error
  void* handle = dlsym(plugin->plugin, func.name);

  const char* error = dlerror();
  if (error != NULL) {
    log_error("Error loading function %s:%s:%s (%s)",
              namespace_name, mod_name, func.name, error);
    return;
  }

  func.function = handle;

  if (registry_add(regman_get_function(), &func) == NULL) {
    log_error("Function %s:%s:%s already registered", namespace_name, mod_name, func.name);
    return;
  }

  log_info("Loading function %s:%s:%s from plugin %s:%s", namespace_name, mod_name, func.name, func.plugin_fid->ns, func.plugin_fid->id);
}

const struct function* function_get(char* name) {
  return registry_ktov(regman_get_function(), &(struct function){.name = name});
}

int function_cmp(const struct function* a, const struct function* b) {
  return registry_strcmp(a->name, b->name);
}

void function_cleanup(struct function* elem) {
  free(elem->name);
}
