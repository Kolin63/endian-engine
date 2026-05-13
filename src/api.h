#ifndef ENDIAN_API_H_
#define ENDIAN_API_H_

#include <concord/discord.h>

#define API_VERSION 1

#ifndef ENDIAN_ENGINE

enum {
  LOG_TRACE,
  LOG_DEBUG,
  LOG_INFO,
  LOG_WARN,
  LOG_ERROR,
  LOG_FATAL
};

#define ENDIAN_DEF_STR_(x) #x
#define ENDIAN_DEF_STR(x) ENDIAN_DEF_STR_(x)

#define log_trace(api, ...) api->log_log(LOG_TRACE, ENDIAN_DEF_STR(MOD_NAME) ":" __FILE_NAME__, __LINE__, __VA_ARGS__)
#define log_debug(api, ...) api->log_log(LOG_DEBUG, ENDIAN_DEF_STR(MOD_NAME) ":" __FILE_NAME__, __LINE__, __VA_ARGS__)
#define log_info(api, ...) api->log_log(LOG_INFO, ENDIAN_DEF_STR(MOD_NAME) ":" __FILE_NAME__, __LINE__, __VA_ARGS__)
#define log_warn(api, ...) api->log_log(LOG_WARN, ENDIAN_DEF_STR(MOD_NAME) ":" __FILE_NAME__, __LINE__, __VA_ARGS__)
#define log_error(api, ...) api->log_log(LOG_ERROR, ENDIAN_DEF_STR(MOD_NAME) ":" __FILE_NAME__, __LINE__, __VA_ARGS__)
#define log_fatal(api, ...) api->log_log(LOG_FATAL, ENDIAN_DEF_STR(MOD_NAME) ":" __FILE_NAME__, __LINE__, __VA_ARGS__)

#endif

struct api {
  int version;

  // void log_log(int level, const char* file, int line, const char* fmt, ...);
  void (*log_log)(int level, const char* file, int line, const char* fmt, ...);

  // attempts exit. if cleanup is not ready, function returns
  //
  // void exit();
  void (*exit)();
  // premature cleanup before everything is initialized, for example to abort
  // the program during the cli args initialization phase
  //
  // void abort_cleanup(int code);
  void (*abort_cleanup)(int code);

  // CCORDcode discord_create_interaction_response(struct discord* client, u64snowflake interaction_id,
  //                                                  const char interaction_token[],
  //                                                  struct discord_interaction_response* params,
  //                                                  struct discord_ret_interaction_response* ret);
  CCORDcode (*discord_create_interaction_response)(struct discord* client, u64snowflake interaction_id,
                                                   const char interaction_token[],
                                                   struct discord_interaction_response* params,
                                                   struct discord_ret_interaction_response* ret);

  // puts a new registry on the heap. registry_cleanup() must be called when it
  // is done being used
  //
  // struct registry* registry_init(int val_size,
  //                                   int (*cmp)(const void*, const void*),
  //                                   void (*cleanup)(void* elem));
  struct registry* (*registry_init)(int val_size,
                                    int (*cmp)(const void*, const void*),
                                    void (*cleanup)(void* elem));

  // frees allocated memory for a registry. if the registry contains structs
  // with data on the heap, those fields must be freed before calling this
  // function
  //
  // void registry_cleanup(struct registry* reg);
  void (*registry_cleanup)(struct registry* reg);

  // calls the registry's cmp function. elides function call if either a or b
  // are NULL. will segfault if the cmp function is unset.
  //
  // int registry_safe_cmp(const struct registry* reg, const void* a, const void* b);
  int (*registry_safe_cmp)(const struct registry* reg, const void* a, const void* b);

  // adds a value. returns pointer to value in registry, or NULL if key already
  // exists
  //
  // void* registry_add(struct registry* reg, const void* val);
  void* (*registry_add)(struct registry* reg, const void* val);

  // removes all entries from registry. does not call registry_cleanup(). does
  // not need to be called before calling registry_cleanup()
  //
  // void registry_clear(struct registry* reg);
  void (*registry_clear)(struct registry* reg);

  // index to value. no bounds checking
  //
  // void* registry_itov(const struct registry* reg, int i);
  void* (*registry_itov)(const struct registry* reg, int i);

  // index to value. returns NULL on error
  //
  // void* registry_itov_safe(const struct registry* reg, int i);
  void* (*registry_itov_safe)(const struct registry* reg, int i);

  // key to index. returns -1 if the key doesn't exist
  //
  // int registry_ktoi(const struct registry* reg, const void* key);
  int (*registry_ktoi)(const struct registry* reg, const void* key);

  // key to value. returns -1 if the key doesn't exist
  //
  // void* registry_ktov(const struct registry* reg, const void* key);
  void* (*registry_ktov)(const struct registry* reg, const void* key);

  // fast implementation of strcmp. only return values are 1, 0, or -1
  //
  // int registry_strcmp(const char* a, const char* b);
  int (*registry_strcmp)(const char* a, const char* b);

  // const struct registry* get_plugin_registry();
  const struct registry* (*get_plugin_registry)();
  // const struct plugin* plugin_get(char* namespace, char* name);
  const struct plugin* (*plugin_get)(char* namespace, char* name);
  // const struct registry* get_function_registry();
  const struct registry* (*get_function_registry)();
  // const struct function* function_get(char* name);
  const struct function* (*function_get)(char* name);
  // const struct registry* get_command_registry();
  const struct registry* (*get_command_registry)();
  // const struct command* command_get(char* name);
  const struct command* (*command_get)(char* name);

  // initializes user
  // returns pointer to user in registry
  //
  // struct user* user_init(unsigned long uuid);
  struct user* (*user_init)(unsigned long uuid);

  // returns pointer to user in registry
  // initializes the user if it is not in registry
  //
  // struct user* user_get(unsigned long uuid);
  struct user* (*user_get)(unsigned long uuid);

  // int user_cmp(struct user* const* a, struct user* const* b);
  int (*user_cmp)(struct user* const* a, struct user* const* b);

  // void user_cleanup(struct user** elem);
  void (*user_cleanup)(struct user** elem);

  // converts uuid (unsigned long) to string.
  // string should be of length UUID_STR_LEN
  //
  // void uuid_to_string(unsigned long uuid, char* buf);
  void (*uuid_to_string)(unsigned long uuid, char* buf);

  // converts string to uuid (unsigned long)
  //
  // unsigned long string_to_uuid(const char* str);
  unsigned long (*string_to_uuid)(const char* str);

  // writes to save file. returns 0 if ok.
  // dir does not need a trailing slash
  // ext is file extension, and it should not include the dot. for example, a
  // json file has the extension "json", not ".json"
  //
  // int save_write(const char* dir, const char* file, const char* ext, const char* content);
  int (*save_write)(const char* dir, const char* file, const char* ext, const char* content);

  // reads from save file into out. returns 0 if ok
  // dir does not need a trailing slash
  // ext is file extension, and it should not include the dot. for example, a
  // json file has the extension "json", not ".json"
  //
  // int save_read(const char* dir, const char* file, const char* ext, char** out);
  int (*save_read)(const char* dir, const char* file, const char* ext, char** out);

  // reads from rom file into out. returns 0 if ok
  // dir does not need a trailing slash
  // ext is file extension, and it should not include the dot. for example, a
  // json file has the extension "json", not ".json"
  //
  // int rom_read(const char* mod, const char* dir, const char* file, const char* ext, char** out);
  int (*rom_read)(const char* mod, const char* dir, const char* file, const char* ext, char** out);
};

#ifdef ENDIAN_ENGINE

void api_init();
void api_cleanup();

const struct api* api_get_global();

// calls all mod functions of type GET_API
void api_call_get_api();
// calls all mod functions of type INIT
void api_call_init();
// calls all mod functions of type LOAD
void api_call_load();
// calls all mod functions of type CLEANUP
void api_call_cleanup();

#endif

#endif
