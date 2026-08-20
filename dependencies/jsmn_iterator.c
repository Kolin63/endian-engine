/*
   jsmn_iterator.c - from the jsmn-iterator library by Colin Melican (Kolin63)

   https://github.com/kolin63/jsmn-iterator

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

#include "jsmn_iterator.h"

#include <stdlib.h>
#include <string.h>

#define JSMN_HEADER
#include <concord/jsmn.h>

void jsmn_iterator_get_string(char* buf, unsigned long size, const char* json, const jsmntok_t* tok) {
  unsigned long len = tok->end - tok->start + 1;
  if (len > size) len = size;
  for (unsigned long i = 0; i < len - 1; i++) {
    buf[i] = json[tok->start + i];
  }
  buf[len - 1] = '\0';
}

char* jsmn_iterator_get_string_heap(const char* json, const jsmntok_t* tok) {
  const unsigned long len = tok->end - tok->start + 1;
  char* str = malloc(len);
  jsmn_iterator_get_string(str, len, json, tok);
  return str;
}

const char* jsmn_iterator_type_to_str(jsmntype_t type) {
  switch (type) {
  case (JSMN_UNDEFINED):
    return "JSMN_UNDEFINED";
  case (JSMN_OBJECT):
    return "JSMN_OBJECT";
  case (JSMN_ARRAY):
    return "JSMN_ARRAY";
  case (JSMN_STRING):
    return "JSMN_STRING";
  case (JSMN_PRIMITIVE):
    return "JSMN_PRIMITIVE";
  }
  return "ERROR - NOT A TYPE";
}

int jsmn_iterator_get_size_recursive(const jsmntok_t* root) {
  int size = 0;
  const jsmntok_t* tok = root + 1;

  for (int i = 0; i < root->size; i++) {
    int diff = jsmn_iterator_get_size_recursive(tok);
    tok += 1 + diff;
    size += 1 + diff;
  }
  return size;
}

void jsmn_iterator_init(struct jsmn_iterator* iter, const jsmntok_t* root, const char* json) {
  iter->i = 0;
  iter->key = NULL;
  iter->val = root;

  iter->root = root;
  iter->json = json;
}

int jsmn_iterator_next(struct jsmn_iterator* iter) {
  if (iter->key != NULL) {
    free(iter->key);
    iter->key = NULL;
  }

  if (iter->i >= iter->root->size) return 0;

  // if it's an object (not an array) the values have keys
  if (iter->root->type == JSMN_OBJECT) {
    const jsmntok_t* key;
    if (iter->i == 0) {
      key = iter->root + 1;
    } else {
      key = iter->val + 1 + jsmn_iterator_get_size_recursive(iter->val);
    }
    iter->key = jsmn_iterator_get_string_heap(iter->json, key);
    iter->val = key + 1;
  } else if (iter->root->type == JSMN_ARRAY) {
    // otherwise, root is an array, so no key
    iter->key = NULL;
    if (iter->i == 0) {
      iter->val = iter->root + 1;
    } else {
      iter->val += 1 + jsmn_iterator_get_size_recursive(iter->val);
    }
  }

  iter->i++;

  return 1;
}
