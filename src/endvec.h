#ifndef ENDIAN_ENDVEC_H_
#define ENDIAN_ENDVEC_H_

#include <stddef.h>
#include <stdlib.h>

#define ENDVEC_DECLARE(name, type)       \
  struct name {                          \
    type* arr;                           \
    size_t len;                          \
    size_t cap;                          \
  };                                     \
                                         \
  void name##_cleanup(struct name* arr); \
  void name##_append(struct name* arr, type val);

#define ENDVEC_DEFINE(name, type, ...)                                            \
  void name##_cleanup(struct name* arr) {                                         \
    if (arr == NULL || arr->arr == NULL)                                          \
      return;                                                                     \
    for (size_t i = 0; i < arr->len; i++) {                                       \
      __VA_ARGS__;                                                                \
    }                                                                             \
    free(arr->arr);                                                               \
    arr->arr = NULL;                                                              \
    arr->len = 0;                                                                 \
    arr->cap = 0;                                                                 \
  }                                                                               \
                                                                                  \
  void name##_append(struct name* arr, type val) {                                \
    arr->len++;                                                                   \
    arr->cap = arr->cap + (arr->len > arr->cap) * arr->cap + (arr->cap == 0) * 1; \
    arr->arr = realloc(arr->arr, arr->cap * sizeof(type));                        \
    arr->arr[arr->len - 1] = val;                                                 \
  }

#endif
