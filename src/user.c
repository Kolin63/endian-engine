#include "user.h"

#include "registry.h"
#include "regman.h"

#include <pthread.h>
#include <concord/discord.h>
#include <concord/discord-response.h>
#include <concord/user.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bot.h"
#include "log.h"

enum user_init_status {
  USER_INIT_STATUS_IDLE,
  USER_INIT_STATUS_WORKING,
  USER_INIT_STATUS_DONE,
  USER_INIT_STATUS_FAIL,
};

static enum user_init_status user_init_status = USER_INIT_STATUS_IDLE;
static pthread_rwlock_t user_lock = PTHREAD_RWLOCK_INITIALIZER;

void user_init_done(struct discord* client, struct discord_response* resp, const struct discord_user* ret) {
  user_init_status = USER_INIT_STATUS_DONE;
}

void user_init_fail(struct discord* client, struct discord_response* resp) {
  user_init_status = USER_INIT_STATUS_FAIL;
}

struct user* user_init(unsigned long uuid) {
  pthread_rwlock_wrlock(&user_lock);

  while (user_init_status != USER_INIT_STATUS_IDLE);
  user_init_status = USER_INIT_STATUS_WORKING;

  struct user* user = malloc(sizeof(struct user));
  user->uuid = uuid;
  if (registry_add(regman_get_user(), &user) == NULL) {
    log_error("Could not initialize user %zi", uuid);
    free(user);
    user_init_status = USER_INIT_STATUS_IDLE;
    pthread_rwlock_unlock(&user_lock);
    return NULL;
  }

  struct discord_user sync = {};
  struct discord_ret_user ret = {.done = user_init_done, .fail = user_init_fail, .sync = &sync};
  discord_get_user(bot_get_global()->discord_bot, uuid, &ret);

  while (user_init_status == USER_INIT_STATUS_WORKING);

  if (user_init_status == USER_INIT_STATUS_FAIL) {
    log_error("Failed to initialize user %zi", uuid);
    free(user);
    user_init_status = USER_INIT_STATUS_IDLE;
    pthread_rwlock_unlock(&user_lock);
    return NULL;
  }

  if (user->uuid != sync.id) {
    log_error("Given UUID does not match returned UUID (%zi)", user->uuid);
    free(user);
    user_init_status = USER_INIT_STATUS_IDLE;
    pthread_rwlock_unlock(&user_lock);
    return NULL;
  }

  user->username = malloc(strlen(sync.username) + 1);
  strcpy(user->username, sync.username);

  char* avatar = malloc(128);
  snprintf(avatar, 128, "https://cdn.discordapp.com/avatars/%zi/%s", uuid, sync.avatar);
  avatar = realloc(avatar, strlen(avatar) + 1);
  user->avatar = avatar;

  discord_user_cleanup(&sync);

  user_init_status = USER_INIT_STATUS_IDLE;

  log_info("Initializing user %s (%zi)", user->username, user->uuid);
  pthread_rwlock_unlock(&user_lock);
  return user;
}

struct user* user_get(unsigned long uuid) {
  pthread_rwlock_rdlock(&user_lock);
  struct user* key = &(struct user){.uuid = uuid};
  struct user** ret_ptr = registry_ktov(regman_get_user(), &key);
  if (ret_ptr == NULL) {
    pthread_rwlock_unlock(&user_lock);
    return user_init(uuid);
  }
  pthread_rwlock_unlock(&user_lock);
  return *ret_ptr;
}

int user_cmp(struct user* const* a, struct user* const* b) {
  const struct user* x = *a;
  const struct user* y = *b;

  if (x == NULL && y == NULL) return 0;
  else if (x == NULL) return -1;
  else if (y == NULL) return 1;

  if (x->uuid > y->uuid) return 1;
  else if (x->uuid < y->uuid) return -1;
  return 0;
}

void user_cleanup(struct user** elem) {
  struct user* user = *elem;
  free(user->username);
  free(user->avatar);
  free(user);
}

void uuid_to_string(unsigned long uuid, char* buf) {
  snprintf(buf, UUID_STR_LEN, "%zi", uuid);
}

unsigned long string_to_uuid(const char* str) {
  return strtoul(str, NULL, 10);
}
