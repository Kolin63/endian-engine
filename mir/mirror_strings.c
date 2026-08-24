#include "mirror_strings.h"

#include <stdlib.h>
#include <log.h>

#include "jsmn_iterator.h"
#include "json_macros.h"

void
mirror_strings_cleanup(struct mirror_strings* arr) {
  if (arr == NULL || arr->len == 0 || arr->arr == NULL) return;
  for (size_t i = 0; i < arr->len; i++) if (arr->arr[i] != NULL) free(arr->arr[i]);
  free(arr->arr);
  arr->arr = NULL;
  arr->len = 0;
}

int
mirror_strings_from_json(struct mirror_strings* arr, const jsmntok_t* jsmn, const char* json) {
  int error = 0;

  arr->arr = NULL;
  arr->len = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_ARRAY_RET(iter, error++; return error);

  while (jsmn_iterator_next(&iter)) {
    END_JSON_CHECK_STRING(iter);
    arr->len++;
    arr->arr = realloc(arr->arr, sizeof(char*) * arr->len);
    char* str = jsmn_iterator_get_string_heap(json, iter.val);
    arr->arr[arr->len - 1] = str;
  }

  return error;
}
