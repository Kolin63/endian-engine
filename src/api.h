#ifndef ENDIAN_API_H_
#define ENDIAN_API_H_

#include <concord/discord.h>

#include "jsmn_iterator.h"

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

#endif

struct registry;

struct api {
  int version;

  // void log_log(int level, const char* file, int line, const char* fmt, ...);
  void (*log_log)(int level, const char* file, int line, const char* fmt, ...);

  // attempts exit. if cleanup is not ready, function returns
  //
  // void handle_exit();
  void (*handle_exit)();
  // server saves all data and tells mods to save all data
  //
  // void handle_save();
  void (*handle_save)();
  // premature cleanup before everything is initialized, for example to abort
  // the program during the cli args initialization phase
  //
  // void abort_cleanup(int code);
  void (*abort_cleanup)(int code);
  // returns 0 if cleanup is not ready, 1 if it is. set to 1 after mod loading
  // and init/load functions are called
  //
  // int get_cleanup_ready();
  int (*get_cleanup_ready)();

  // CCORDcode discord_create_interaction_response(struct discord* client, u64snowflake interaction_id,
  //                                                  const char interaction_token[],
  //                                                  struct discord_interaction_response* params,
  //                                                  struct discord_ret_interaction_response* ret);
  CCORDcode (*discord_create_interaction_response)(struct discord* client, u64snowflake interaction_id,
                                                   const char interaction_token[],
                                                   struct discord_interaction_response* params,
                                                   struct discord_ret_interaction_response* ret);

  // initializes a registry. registry_cleanup() must be called when it is done
  // being used
  // void registry_init(struct registry* reg, int val_size,
  //                    int (*cmp)(const void*, const void*),
  //                    void (*cleanup)(void* elem));
  void (*registry_init)(struct registry* reg, int val_size,
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

  // const struct registry* get_namespace_registry();
  const struct registry* (*get_namespace_registry)();
  // const struct namespace* namespace_get(const char* name);
  const struct namespace* (*namespace_get)(const char* name);
  // const struct namespace_mod_entry* namespace_mod_entry_get(const struct namespace* ns, const char* name);
  const struct namespace_mod_entry* (*namespace_mod_entry_get)(const struct namespace* ns, const char* name);
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

  // gets the string value of a JSMN token and puts it in a string buffer. size is
  // the length of the buffer, including the null terminator. json is the string
  // of the json content, and the same string that is passed to jsmn_parse()
  //
  // void jsmn_iterator_get_string(char* buf, unsigned long size, const char* json, const jsmntok_t* tok);
  void (*jsmn_iterator_get_string)(char* buf, unsigned long size, const char* json, const jsmntok_t* tok);

  // gets the string value of a JSMN token and puts it on the heap. the string
  // must be freed after use. json is the string of the json content, and the same
  // string that is passed to jsmn_parse()
  //
  // char* jsmn_iterator_get_string_heap(const char* json, const jsmntok_t* tok);
  char* (*jsmn_iterator_get_string_heap)(const char* json, const jsmntok_t* tok);

  // converts JSMN integer error code into a string
  // return values:
  //       0: "JSMN_UNDEFINED"
  //       1: "JSMN_OBJECT"
  //       2: "JSMN_ARRAY"
  //       4: "JSMN_STRING"
  //       8: "JSMN_PRIMITIVE"
  // default: "ERROR - NOT A TYPE"
  //
  // const char* jsmn_iterator_type_to_str(jsmntype_t type);
  const char* (*jsmn_iterator_type_to_str)(jsmntype_t type);

  // root is the root object/array to iterate over. json is the string of the json
  // content, and the same string that is passed to jsmn_parse()
  //
  // void jsmn_iterator_init(struct jsmn_iterator* iter, const jsmntok_t* root, const char* json);
  void (*jsmn_iterator_init)(struct jsmn_iterator* iter, const jsmntok_t* root, const char* json);

  // iterates to next value. returns 1 if iteration can continue, and 0 if not
  //
  // int jsmn_iterator_next(struct jsmn_iterator* iter);
  int (*jsmn_iterator_next)(struct jsmn_iterator* iter);

  // Moves all contents of file into a buffer. buf must be freed after use
  // char* fileio_read_all(FILE* file);
  char* (*fileio_read_all)(FILE* file);

  // jsmntok_t* fileio_read_json(const char* json);
  jsmntok_t* (*fileio_read_json)(const char* json);

  // struct bot* bot_get_global();
  struct bot* (*bot_get_global)();

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
// calls all mod functions of type SAVE
void api_call_save();
// calls all mod functions of type CLEANUP
void api_call_cleanup();

#endif

#endif
