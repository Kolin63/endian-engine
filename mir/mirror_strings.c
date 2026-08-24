#include "mirror_strings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <log.h>

#include "jsmn_iterator.h"
#include "json_macros.h"

void
mirror_strings_remove_backslashes(struct mirror_strings* arr) {
  for (size_t j = 0; j < arr->len; j++) {
    char* str = arr->arr[j];
    if (str == NULL) continue;

    const size_t len = strlen(str);

    char* buf = malloc(len + 1);
    strcpy(buf, str);
    char* ptr = buf;

    bool escape = false;
    for (size_t i = 0; i < len; i++) {
      if (str[i] == '\\' && escape == false) {
        escape = true;
        continue;
      }

      escape = false;

      *ptr = str[i];
      ptr++;
    }

    *ptr = '\0';
    strcpy(str, buf);
    free(buf);
  }
}

void
mirror_strings_append_newline_to_all(struct mirror_strings* arr) {
  for (size_t i = 0; i < arr->len; i++) {
    if (arr->arr[i] == NULL) continue;
    const size_t len = strlen(arr->arr[i]) + 1;
    arr->arr[i] = realloc(arr->arr[i], len + 1);
    strcat(arr->arr[i], "\n");
  }
}

void
mirror_strings_cleanup(struct mirror_strings* arr) {
  if (arr == NULL || arr->len == 0 || arr->arr == NULL) return;
  for (size_t i = 0; i < arr->len; i++)
    if (arr->arr[i] != NULL) free(arr->arr[i]);
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

int
mirror_strings_write(FILE* file, const struct mirror_strings* arr) {
  int error = 0;

  for (size_t i = 0; i < arr->len; i++) {
    if (arr->arr[i] == NULL) continue;
    if (fputs(arr->arr[i], file) == EOF) {
      log_error("Could not write mirror_strings to file");
      error++;
    }
  }

  return error;
}
