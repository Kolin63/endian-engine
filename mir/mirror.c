#include "mirror.h"

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#define JSMN_HEADER
#include <log.h>
#include <concord/jsmn.h>

#include "jsmn_iterator.h"
#include "json_macros.h"
#include "mod_stack.h"
#include "../src/fileio.h"

void mirror_foreach_cleanup(struct mirror_foreach* f) {
  free(f->tag);
  mirror_strings_cleanup(&f->format);
}

int mirror_foreach_from_json(struct mirror_foreach* f, const jsmntok_t* jsmn, const char* json) {
  int error = 0;

  f->tag = NULL;
  f->format.arr = NULL;
  f->format.len = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_OBJECT_RET(iter, error++; return error);

  while (jsmn_iterator_next(&iter)) {
    if (strcmp(iter.key, "tag") == 0) {
      END_JSON_CHECK_STRING(iter);
      f->tag = jsmn_iterator_get_string_heap(json, iter.val);
    } else if (strcmp(iter.key, "format") == 0) {
      END_JSON_CHECK_ARRAY(iter);
      error += mirror_strings_from_json(&f->format, iter.val, json);
    } else {
      error++;
      log_error(MOD_STACK_FMT "Unknown object %s", MOD_STACK_ARG, iter.key);
    }
  }

  return error;
}

void mirror_foreach_arr_cleanup(struct mirror_foreach_arr* arr) {
  if (arr->len == 0 || arr == NULL) return;
  for (size_t i = 0; i < arr->len; i++) mirror_foreach_cleanup(&arr->arr[i]);
  free(arr->arr);
  arr->arr = NULL;
  arr->len = 0;
}

int mirror_foreach_arr_from_json(struct mirror_foreach_arr* arr, const jsmntok_t* jsmn, const char* json) {
  int error = 0;

  arr->arr = NULL;
  arr->len = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_ARRAY_RET(iter, error++; return error);

  while (jsmn_iterator_next(&iter)) {
    arr->len++;
    arr->arr = realloc(arr->arr, sizeof(struct mirror_foreach) * arr->len);
    error += mirror_foreach_from_json(&arr->arr[arr->len - 1], iter.val, json);
  }

  return error;
}

void mirror_group_cleanup(struct mirror_group* g) {
  if (g == NULL) return;
  mirror_strings_cleanup(&g->prefix);
  mirror_foreach_arr_cleanup(&g->foreach);
  mirror_strings_cleanup(&g->postfix);
}

int mirror_group_from_json(struct mirror_group* g, const jsmntok_t* jsmn, const char* json) {
  int error = 0;

  g->prefix.arr = NULL;
  g->prefix.len = 0;
  g->foreach.arr = NULL;
  g->foreach.len = 0;
  g->postfix.arr = NULL;
  g->postfix.len = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_OBJECT_RET(iter, error++; return error);

  while (jsmn_iterator_next(&iter)) {
    if (strcmp(iter.key, "prefix") == 0) {
      END_JSON_CHECK_ARRAY(iter);
      error += mirror_strings_from_json(&g->prefix, iter.val, json);
    } else if (strcmp(iter.key, "foreach") == 0) {
      END_JSON_CHECK_ARRAY(iter);
      error += mirror_foreach_arr_from_json(&g->foreach, iter.val, json);
    } else if (strcmp(iter.key, "postfix") == 0) {
      END_JSON_CHECK_ARRAY(iter);
      error += mirror_strings_from_json(&g->postfix, iter.val, json);
    } else {
      error++;
      log_error(MOD_STACK_FMT "Unknown object %s", MOD_STACK_ARG, iter.key);
    }
  }

  return error;
}

void mirror_groups_cleanup(struct mirror_groups* arr) {
  if (arr->len == 0 || arr == NULL) return;
  for (size_t i = 0; i < arr->len; i++) mirror_group_cleanup(&arr->arr[i]);
  free(arr->arr);
  arr->arr = NULL;
  arr->len = 0;
}

int mirror_groups_from_json(struct mirror_groups* arr, const jsmntok_t* jsmn, const char* json) {
  int error = 0;

  arr->arr = NULL;
  arr->len = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_ARRAY_RET(iter, error++; return error);

  while (jsmn_iterator_next(&iter)) {
    arr->len++;
    arr->arr = realloc(arr->arr, sizeof(struct mirror_group) * arr->len);
    error += mirror_group_from_json(&arr->arr[arr->len - 1], iter.val, json);
  }

  return error;
}

void mirror_file_cleanup(struct mirror_file* f) {
  if (f == NULL) return;
  free(f->name);
  mirror_groups_cleanup(&f->groups);
}

int mirror_file_from_json(struct mirror_file* f, const jsmntok_t* jsmn, const char* json) {
  int error = 0;

  f->name = NULL;
  f->groups.arr = NULL;
  f->groups.len = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_OBJECT_RET(iter, error++; return error);

  while (jsmn_iterator_next(&iter)) {
    if (strcmp(iter.key, "name") == 0) {
      END_JSON_CHECK_STRING(iter);
      f->name = jsmn_iterator_get_string_heap(json, iter.val);
    } else if (strcmp(iter.key, "groups") == 0) {
      END_JSON_CHECK_ARRAY(iter);
      error += mirror_groups_from_json(&f->groups, iter.val, json);
    } else {
      error++;
      log_error(MOD_STACK_FMT "Unknown object %s", MOD_STACK_ARG, iter.key);
    }
  }

  return error;
}

void mirror_files_cleanup(struct mirror_files* arr) {
  if (arr->len == 0 || arr == NULL) return;
  for (size_t i = 0; i < arr->len; i++) mirror_file_cleanup(&arr->arr[i]);
  free(arr->arr);
  arr->arr = NULL;
  arr->len = 0;
}

int mirror_files_from_json(struct mirror_files* arr, const jsmntok_t* jsmn, const char* json) {
  int error = 0;

  arr->arr = NULL;
  arr->len = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_ARRAY_RET(iter, error++; return error);

  while (jsmn_iterator_next(&iter)) {
    arr->len++;
    arr->arr = realloc(arr->arr, sizeof(struct mirror_file) * arr->len);
    error += mirror_file_from_json(&arr->arr[arr->len - 1], iter.val, json);
  }

  return error;
}

void mirror_cleanup(struct mirror* m) {
  if (m == NULL) return;
  mirror_files_cleanup(&m->files);
}

int mirror_from_json(struct mirror* m, const jsmntok_t* jsmn, const char* json) {
  int error = 0;

  m->files.arr = NULL;
  m->files.len = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_OBJECT_RET(iter, error++; return error);

  while (jsmn_iterator_next(&iter)) {
    if (strcmp(iter.key, "files") == 0) {
      END_JSON_CHECK_ARRAY(iter);
      error += mirror_files_from_json(&m->files, iter.val, json);
    } else {
      error++;
      log_error(MOD_STACK_FMT "Unknown object %s", MOD_STACK_ARG, iter.key);
    }
  }

  return error;
}

void mirror_load(const char* file_path) {
  if (strcmp(mod_stack.file, "template.json") == 0) return;

  FILE* file = fopen(file_path, "r");

  if (!file) {
    log_error(MOD_STACK_FMT "Could not open mirror file (%s)", MOD_STACK_ARG, file_path);
    return;
  }

  char* json = fileio_read_all(file);
  fclose(file);

  jsmntok_t* jsmn = fileio_read_json(json);

  struct mirror m;
  if (mirror_from_json(&m, jsmn, json) != 0) {
    free(json);
    free(jsmn);
    return;
  }

  free(json);
  free(jsmn);

  mirrors.len++;
  mirrors.arr = realloc(mirrors.arr, sizeof(struct mirror) * mirrors.len);
  mirrors.arr[mirrors.len - 1] = m;

  log_info("Loading mirror %s", mod_stack.file);
}

void mirrors_cleanup(struct mirrors* arr) {
  if (arr->len == 0 || arr == NULL) return;
  free(arr->arr);
  arr->arr = NULL;
  arr->len = 0;
}
