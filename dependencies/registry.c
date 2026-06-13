/*
   registry.c - part of the registry.c library by Colin Melican (Kolin63)

   https://github.com/kolin63/registry.c

   Under the MIT License

   Copyright (c) 2026 Colin Melican

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
 */

#include "registry.h"

#include <stdlib.h>
#include <string.h>

void registry_init(struct registry* reg, int val_size,
                   int (*cmp)(const void*, const void*),
                   void (*cleanup)(void*)) {
  reg->length = 0;
  reg->val_size = val_size;
  reg->cmp = cmp;
  reg->cleanup = cleanup;
  reg->vals = NULL;
}

void registry_value_cleanup(const struct registry* reg) {
  if (reg->cleanup != NULL) {
    for (int i = 0; i < reg->length; i++) {
      reg->cleanup(registry_itov(reg, i));
    }
  }
  free(reg->vals);
}

void registry_cleanup(struct registry* reg) {
  registry_value_cleanup(reg);
}

int registry_safe_cmp(const struct registry* reg, const void* a,
                      const void* b) {
  if (a == b) return 0;
  else if (a == NULL || b == NULL) {
    if (a > b) return 1;
    else if (a < b) return -1;
  }
  return reg->cmp(a, b);
}

void* registry_add(struct registry* reg, const void* val) {
  // do a binary search to find insertion index
  // 0 2 4 6 8
  //       ^
  //       5 (insert at index 3)
  int insert_index = 0;
  {
    int left = 0;
    int right = reg->length - 1;
    while (left <= right) {
      int mid = left + (right - left) / 2;
      int cmp = registry_safe_cmp(reg, reg->vals + mid * reg->val_size, val);
      if (cmp < 0) {
        left = mid + 1;
      } else if (cmp > 0) {
        right = mid - 1;
      } else {
        return NULL;
      }
    }
    insert_index = left + (right - left) / 2;
  }

  // move values to make room
  // 0 2 4 _ 6 8
  //       ^
  //       5
  {
    reg->vals = realloc(reg->vals, (reg->length + 1) * reg->val_size);
    void* src = reg->vals + insert_index * reg->val_size;
    void* dest = src + reg->val_size;
    const size_t n = (reg->length - insert_index) * reg->val_size;
    memmove(dest, src, n);
  }

  // move in new value
  {
    const void* src = val;
    void* dest = reg->vals + insert_index * reg->val_size;
    const size_t n = reg->val_size;
    memcpy(dest, src, n);
  }

  reg->length++;
  return registry_itov(reg, insert_index);
}

int registry_del(struct registry* reg, int i, void* val) {
  int next_len = reg->length - i - 1;
  if (next_len > 0) {
    const void* next = registry_itov(reg, i + 1);
    memmove(val, next, next_len * reg->val_size);
  }

  reg->length--;
  if (reg->length == 0) {
    free(reg->vals);
    reg->vals = NULL;
  } else {
    reg->vals = realloc(reg->vals, reg->length * reg->val_size);
  }

  return 0;
}

int registry_del_val(struct registry* reg, void* val) {
  int i = registry_vtoi(reg, val);
  return registry_del(reg, i, val);
}

int registry_del_key(struct registry* reg, const void* key) {
  int i = registry_ktoi(reg, key);
  if (i == -1) return -1;
  void* val = registry_itov(reg, i);
  return registry_del(reg, i, val);
}

int registry_del_i(struct registry* reg, int i) {
  void* val = registry_itov(reg, i);
  return registry_del(reg, i, val);
}

void registry_clear(struct registry* reg) {
  registry_value_cleanup(reg);
  reg->vals = NULL;
  reg->length = 0;
}

void* registry_itov(const struct registry* reg, int i) {
  return reg->vals + i * reg->val_size;
}

void* registry_itov_safe(const struct registry* reg, int i) {
  if (i < 0 || i >= reg->length) {
    return NULL;
  } else {
    return reg->vals + i * reg->val_size;
  }
}

int registry_ktoi(const struct registry* reg, const void* key) {
  int left = 0;
  int right = reg->length - 1;
  while (left <= right) {
    int mid = left + (right - left) / 2;
    int cmp = registry_safe_cmp(reg, reg->vals + mid * reg->val_size, key);
    if (cmp < 0) {
      left = mid + 1;
    } else if (cmp > 0) {
      right = mid - 1;
    } else {
      return mid;
    }
  }
  return -1;
}

void* registry_ktov(const struct registry* reg, const void* key) {
  int i = registry_ktoi(reg, key);
  if (i < 0) return NULL;
  return registry_itov(reg, i);
}

int registry_vtoi(const struct registry* reg, const void* val) {
  return (val - reg->vals) / reg->val_size;
}

int registry_strcmp(const char* a, const char* b) {
  while (1) {
    if (*a > *b) return 1;
    else if (*a < *b) return -1;
    else if (*a == '\0') return 0;
    a++;
    b++;
  }
}
