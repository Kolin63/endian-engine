#include "regman.h"

#include <stdlib.h>

#include "user.h"
#include "registry.h"

struct regman* global;

void
regman_init() {
  global = malloc(sizeof(struct regman));

  registry_init(&(global->user), sizeof(struct user*), (void*)user_cmp, (void*)user_cleanup);
}

void
regman_cleanup() {
  registry_cleanup(&(global->user));
  free(global);
}

struct regman*
regman_get() { return global; }

struct registry*
regman_get_user() { return &(global->user); }
