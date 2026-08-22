#ifndef ENDIAN_JSON_MACROS_H_
#define ENDIAN_JSON_MACROS_H_

#include "mod_stack.h"

#define JSMN_HEADER
#include <concord/jsmn.h>

// checks that a bool is the proper type. custom return
// params:
// - const jsmn_iterator _iter
#define END_JSON_CHECK_BOOL_RET(_iter, _ret)                                       \
  if (_iter.val->type != JSMN_PRIMITIVE) {                                         \
    log_error(MOD_STACK_FMT "%s must be bool (type)", MOD_STACK_ARG, _iter.key);   \
    _ret;                                                                          \
  }                                                                                \
  {                                                                                \
    char val[2];                                                                   \
    jsmn_iterator_get_string(val, 2, _iter.json, _iter.val);                       \
    if (val[0] != 'f' && val[0] != 't') {                                          \
      log_error(MOD_STACK_FMT "%s must be bool (char)", MOD_STACK_ARG, _iter.key); \
      _ret;                                                                        \
    }                                                                              \
  }

// checks that a bool is the proper type.
// params:
// - const jsmn_iterator _iter
// assumes that the following variables exist:
// - int error
#define END_JSON_CHECK_BOOL(_iter) END_JSON_CHECK_BOOL_RET(_iter, error++; continue);

// checks that a null value is the proper type. custom return
// params:
// - const jsmn_iterator _iter
#define END_JSON_CHECK_NULL_RET(_iter, _ret)                                       \
  if (_iter.val->type != JSMN_PRIMITIVE) {                                         \
    log_error(MOD_STACK_FMT "%s must be null (type)", MOD_STACK_ARG, _iter.key);   \
    _ret;                                                                          \
  }                                                                                \
  {                                                                                \
    char val[2];                                                                   \
    jsmn_iterator_get_string(val, 2, _iter.json, _iter.val);                       \
    if (val[0] != 'n') {                                                           \
      log_error(MOD_STACK_FMT "%s must be null (char)", MOD_STACK_ARG, _iter.key); \
      _ret;                                                                        \
    }                                                                              \
  }

// checks that a null value is the proper type.
// params:
// - const jsmn_iterator _iter
// assumes that the following variables exist:
// - int error
#define END_JSON_CHECK_NULL(_iter) END_JSON_CHECK_NULL_RET(_iter, error++; continue);

// checks that a number is the proper type. custom return
// params:
// - const jsmn_iterator _iter
#define END_JSON_CHECK_NUMBER_RET(_iter, _ret)                                       \
  if (_iter.val->type != JSMN_PRIMITIVE) {                                           \
    log_error(MOD_STACK_FMT "%s must be number (type)", MOD_STACK_ARG, _iter.key);   \
    _ret;                                                                            \
  }                                                                                  \
  {                                                                                  \
    char val[2];                                                                     \
    jsmn_iterator_get_string(val, 2, _iter.json, _iter.val);                         \
    if (val[0] != '-' && (val[0] < '0' || val[0] > '9')) {                           \
      log_error(MOD_STACK_FMT "%s must be number (char)", MOD_STACK_ARG, _iter.key); \
      _ret;                                                                          \
    }                                                                                \
  }

// checks that a number is the proper type
// params:
// - const jsmn_iterator _iter
// assumes that the following variables exist:
// - int error
#define END_JSON_CHECK_NUMBER(_iter) END_JSON_CHECK_NUMBER_RET(_iter, error++; continue);

// checks that a string is the proper type. custom return
// params:
// - const jsmn_iterator _iter
#define END_JSON_CHECK_STRING_RET(_iter, _ret)                              \
  if (_iter.val->type != JSMN_STRING) {                                     \
    log_error(MOD_STACK_FMT "%s must be string", MOD_STACK_ARG, _iter.key); \
    _ret;                                                                   \
  }

// checks that a string is the proper type
// params:
// - const jsmn_iterator _iter
// assumes that the following variables exist:
// - int error
#define END_JSON_CHECK_STRING(_iter) END_JSON_CHECK_STRING_RET(_iter, error++; continue);

// checks that string length is: min <= length <= max. custom return
// params:
// - const jsmn_iterator _iter
#define END_JSON_CHECK_STRING_LENGTH_RET(_iter, _ret, min, max)              \
  {                                                                          \
    int len = _iter.val->end - _iter.val->start;                             \
    if (len < min || len > max) {                                            \
      log_error(MOD_STACK_FMT "%s is wrong size", MOD_STACK_ARG, _iter.key); \
      _ret;                                                                  \
    }                                                                        \
  }

// checks that string length is: min <= length <= max
// params:
// - const jsmn_iterator _iter
// assumes that the following variables exist:
// - int error
#define END_JSON_CHECK_STRING_LENGTH(_iter, min, max) END_JSON_CHECK_STRING_LENGTH_RET(_iter, error++; continue, min, max);

// checks that an array is the proper type. custom return
// params:
// - const jsmn_iterator _iter
#define END_JSON_CHECK_ARRAY_RET(_iter, _ret)                              \
  if (_iter.val->type != JSMN_ARRAY) {                                     \
    log_error(MOD_STACK_FMT "%s must be array", MOD_STACK_ARG, _iter.key); \
    _ret;                                                                  \
  }

// checks that an array is the proper type
// params:
// - const jsmn_iterator _iter
// assumes that the following variables exist:
// - int error
#define END_JSON_CHECK_ARRAY(_iter) END_JSON_CHECK_ARRAY_RET(_iter, error++; continue);

// checks that an array is the proper type and has at least one child. custom
// return
// params:
// - const jsmn_iterator _iter
#define END_JSON_CHECK_ARRAY_AND_CHILDREN_RET(_iter, _ret)                   \
  END_JSON_CHECK_ARRAY_RET(_iter, _ret);                                     \
  if (_iter.val->size == 0) {                                                \
    log_error(MOD_STACK_FMT "%s has no children", MOD_STACK_ARG, _iter.key); \
    _ret;                                                                    \
  }

// checks that an array is the proper type and has at least one child
// params:
// - const jsmn_iterator _iter
// assumes that the following variables exist:
// - int error
#define END_JSON_CHECK_ARRAY_AND_CHILDREN(_iter) END_JSON_CHECK_ARRAY_AND_CHILDREN_RET(_iter, error++; continue);

// checks that an object is the proper type. custom return
// params:
// - const jsmn_iterator _iter
#define END_JSON_CHECK_OBJECT_RET(_iter, _ret)                              \
  if (_iter.val->type != JSMN_OBJECT) {                                     \
    log_error(MOD_STACK_FMT "%s must be object", MOD_STACK_ARG, _iter.key); \
    _ret;                                                                   \
  }

// checks that an object is the proper type
// params:
// - const jsmn_iterator _iter
// assumes that the following variables exist:
// - int error
#define END_JSON_CHECK_OBJECT(_iter) END_JSON_CHECK_OBJECT_RET(_iter, error++; continue);

// checks that an object is the proper type and has at least one child. custom
// return
// params:
// - const jsmn_iterator _iter
#define END_JSON_CHECK_OBJECT_AND_CHILDREN_RET(_iter, _ret)                  \
  END_JSON_CHECK_OBJECT_RET(_iter, _ret);                                    \
  if (_iter.val->size == 0) {                                                \
    log_error(MOD_STACK_FMT "%s has no children", MOD_STACK_ARG, _iter.key); \
    _ret;                                                                    \
  }

// checks that an object is the proper type and has at least one child
// params:
// - const jsmn_iterator _iter
// assumes that the following variables exist:
// - int error
#define END_JSON_CHECK_OBJECT_AND_CHILDREN(_iter) END_JSON_CHECK_OBJECT_AND_CHILDREN_RET(_iter, error++; continue);

#endif
