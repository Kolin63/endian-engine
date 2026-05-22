/*
   registry.h - part of the registry.c library by Colin Melican (Kolin63)

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

#ifndef COLIN_REGISTRY_H_
#define COLIN_REGISTRY_H_

#define COLIN_REGISTRY_VERSION_MAJOR 2
#define COLIN_REGISTRY_VERSION_MINOR 0
#define COLIN_REGISTRY_VERSION_PATCH 0

// an interface to a single registry. intialized with registry_init(). none of
// these fields should be manually written to.
struct registry {
  // amount of keys / values
  int length;

  // size of value type in bytes
  int val_size;

  // comparison function for data type. does not need to check if values are
  // null. should return:
  // * 0 if a == b
  // * a negative value if a < b
  // * a positive value if a > b
  int (*cmp)(const void* a, const void* b);

  // cleanup function that will be called whenever a value is removed from the
  // registry. the argument elem is a pointer to the element that will be freed
  // this should be used if, for example, there is a struct that has a pointer
  // to something on the heap. if this is NULL, it will not be called
  void (*cleanup)(void* elem);

  // value data. continguous in memory
  void* vals;
};

// initializes a registry. registry_cleanup() must be called when it is done
// being used
void registry_init(struct registry* reg, int val_size,
                   int (*cmp)(const void*, const void*),
                   void (*cleanup)(void* elem));

// frees allocated memory for a registry. if the registry contains structs with
// data on the heap, those fields must be freed before calling this function
void registry_cleanup(struct registry* reg);

// calls the registry's cmp function. elides function call if either a or b are
// NULL. will segfault if the cmp function is unset.
int registry_safe_cmp(const struct registry* reg, const void* a, const void* b);

// adds a value. returns pointer to value in registry, or NULL if key already
// exists
void* registry_add(struct registry* reg, const void* val);

// removes all entries from registry. does not call registry_cleanup(). does not
// need to be called before calling registry_cleanup()
void registry_clear(struct registry* reg);

// index to value. no bounds checking
void* registry_itov(const struct registry* reg, int i);

// index to value. returns NULL on error
void* registry_itov_safe(const struct registry* reg, int i);

// key to index. returns -1 if the key doesn't exist
int registry_ktoi(const struct registry* reg, const void* key);

// key to value. returns NULL if the key doesn't exist
void* registry_ktov(const struct registry* reg, const void* key);

// fast implementation of strcmp. only return values are 1, 0, or -1
int registry_strcmp(const char* a, const char* b);

#endif
