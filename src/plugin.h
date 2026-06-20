#ifndef ENDIAN_PLUGIN_H_
#define ENDIAN_PLUGIN_H_

#include "fid.h"

struct plugin {
  void* plugin;
  struct fid fid;
};

void plugin_load(const char* plugin_path, const char* namespace_name, const char* mod_name, const char* plugin_name);
const struct plugin* plugin_get(const struct fid* fid);

int plugin_cmp(const struct plugin* a, const struct plugin* b);

void plugin_cleanup(struct plugin* elem);

#endif
