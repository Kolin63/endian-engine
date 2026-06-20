#include "fid.h"

#include <stddef.h>

struct fid fid_split(char* str) {
  char* colon = str;
  while (*colon != ':' && *colon != '\0') colon++;

  if (*colon == '\0') return (struct fid){.ns = NULL, .id = str};

  *colon = '\0';
  return (struct fid){.ns = str, .id = colon + 1};
}
