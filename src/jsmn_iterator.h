/*
   jsmn_iterator.h - from the jsmn-iterator library by Colin Melican (Kolin63)

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

#ifndef KOLIN_JSMN_ITERATOR_H_
#define KOLIN_JSMN_ITERATOR_H_

#define JSMN_STATIC
#include <concord/jsmn.h>

// used to iterate through JSMN tokens. initialized with jsmn_iterator_init().
// do not write to any of these fields
struct jsmn_iterator {
  // index of value. root object/array is 0
  unsigned int i;

  // string value of key. NULL if root is an array
  char* key;

  // token of value. to get string value, use jsmn_iterator_get_string() or
  // jsmn_iterator_get_string_heap()
  const jsmntok_t* val;

  // token of root object / array
  const jsmntok_t* root;

  // string of json content. the same string that is passed to jsmn_parse()
  const char* json;
};

#ifdef ENDIAN_ENGINE

// gets the string value of a JSMN token and puts it in a string buffer. size is
// the length of the buffer, including the null terminator. json is the string
// of the json content, and the same string that is passed to jsmn_parse()
void jsmn_iterator_get_string(char* buf, unsigned long size, const char* json, const jsmntok_t* tok);

// gets the string value of a JSMN token and puts it on the heap. the string
// must be freed after use. json is the string of the json content, and the same
// string that is passed to jsmn_parse()
char* jsmn_iterator_get_string_heap(const char* json, const jsmntok_t* tok);

// converts JSMN integer error code into a string
// return values:
//       0: "JSMN_UNDEFINED"
//       1: "JSMN_OBJECT"
//       2: "JSMN_ARRAY"
//       4: "JSMN_STRING"
//       8: "JSMN_PRIMITIVE"
// default: "ERROR - NOT A TYPE"
const char* jsmn_iterator_type_to_str(jsmntype_t type);

// root is the root object/array to iterate over. json is the string of the json
// content, and the same string that is passed to jsmn_parse()
void jsmn_iterator_init(struct jsmn_iterator* iter, const jsmntok_t* root, const char* json);

// iterates to next value. returns 1 if iteration can continue, and 0 if not
int jsmn_iterator_next(struct jsmn_iterator* iter);

#endif

#endif
