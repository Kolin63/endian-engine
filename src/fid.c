#include "fid.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct fid fid_split(char* str) {
  char* colon = str;
  while (*colon != ':' && *colon != '\0') colon++;

  if (*colon == '\0') return (struct fid){.ns = NULL, .id = str};

  *colon = '\0';
  return (struct fid){.ns = str, .id = colon + 1};
}

char* fid_to_json(const struct fid* fid) {
  // 7      x  8       x  2
  // {"ns":"...","id":"..."}
  char* str = malloc(7 + strlen(fid->ns) + 8 + strlen(fid->id) + 2 + 1);

  strcpy(str, "{\"ns\":\"");
  strcat(str, fid->ns);
  strcat(str, "\",\"id\":\"");
  strcat(str, fid->id);
  strcat(str, "\"}");

  return str;
}
