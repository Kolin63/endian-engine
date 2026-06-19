#ifndef ENDIAN_JSON_MACROS_H_
#define ENDIAN_JSON_MACROS_H_

// checks that a bool is the proper type. custom return
// params:
// - const jsmn_iterator _iter
// assumes the following variables exist:
// - struct json_iterator* iter
// - const char* json
// - const char* file_name
// - const char* mod_name
#define END_JSON_CHECK_BOOL_RET(_iter, _ret)                                                            \
  if (_iter.val->type != JSMN_PRIMITIVE) {                                                              \
    log_error("In data type %s from mod %s, %s must be bool (type)", file_name, mod_name, _iter.key);   \
    _ret;                                                                                               \
  }                                                                                                     \
  {                                                                                                     \
    char val[2];                                                                                        \
    jsmn_iterator_get_string(val, 2, json, _iter.val);                                                  \
    if (val[0] != 'f' && val[0] != 't') {                                                               \
      log_error("In data type %s from mod %s, %s must be bool (char)", file_name, mod_name, _iter.key); \
      _ret;                                                                                             \
    }                                                                                                   \
  }

// checks that a bool is the proper type.
// params:
// - const jsmn_iterator _iter
// assumes the following variables exist:
// - struct json_iterator* iter
// - const char* json
// - const char* file_name
// - const char* mod_name
// - int error
#define END_JSON_CHECK_BOOL(_iter) END_JSON_CHECK_BOOL_RET(_iter, error++; continue);

// checks that a null value is the proper type. custom return
// params:
// - const jsmn_iterator _iter
// assumes the following variables exist:
// - struct json_iterator* iter
// - const char* json
// - const char* file_name
// - const char* mod_name
#define END_JSON_CHECK_NULL_RET(_iter, _ret)                                                            \
  if (_iter.val->type != JSMN_PRIMITIVE) {                                                              \
    log_error("In data type %s from mod %s, %s must be null (type)", file_name, mod_name, _iter.key);   \
    _ret;                                                                                               \
  }                                                                                                     \
  {                                                                                                     \
    char val[2];                                                                                        \
    jsmn_iterator_get_string(val, 2, json, _iter.val);                                                  \
    if (val[0] != 'n') {                                                                                \
      log_error("In data type %s from mod %s, %s must be null (char)", file_name, mod_name, _iter.key); \
      _ret;                                                                                             \
    }                                                                                                   \
  }

// checks that a null value is the proper type.
// params:
// - const jsmn_iterator _iter
// assumes the following variables exist:
// - struct json_iterator* iter
// - const char* json
// - const char* file_name
// - const char* mod_name
// - int error
#define END_JSON_CHECK_NULL(_iter) END_JSON_CHECK_NULL_RET(_iter, error++; continue);

// checks that a number is the proper type. custom return
// params:
// - const jsmn_iterator _iter
// assumes the following variables exist:
// - struct json_iterator* iter
// - const char* json
// - const char* file_name
// - const char* mod_name
#define END_JSON_CHECK_NUMBER_RET(_iter, _ret)                                                          \
  if (_iter.val->type != JSMN_PRIMITIVE) {                                                              \
    log_error("In data type %s from mod %s, %s must be number (type)", file_name, mod_name, _iter.key); \
    _ret;                                                                                               \
  }                                                                                                     \
  {                                                                                                     \
    char val[2];                                                                                        \
    jsmn_iterator_get_string(val, 2, json, _iter.val);                                                  \
    if (val[0] != '-' && (val[0] < '0' || val[0] > '9')) {                                              \
      log_error("In data type %s from mod %s, %s must be bool (char)", file_name, mod_name, _iter.key); \
      _ret;                                                                                             \
    }                                                                                                   \
  }

// checks that a number is the proper type
// params:
// - const jsmn_iterator _iter
// assumes the following variables exist:
// - struct json_iterator* iter
// - const char* json
// - const char* file_name
// - const char* mod_name
// - int error
#define END_JSON_CHECK_NUMBER(_iter) END_JSON_CHECK_NUMBER_RET(_iter, error++; continue);

// checks that a string is the proper type. custom return
// params:
// - const jsmn_iterator _iter
// assumes the following variables exist:
// - const char* file_name
// - const char* mod_name
#define END_JSON_CHECK_STRING_RET(_iter, _ret)                                                   \
  if (_iter.val->type != JSMN_STRING) {                                                          \
    log_error("In data type %s from mod %s, %s must be string", file_name, mod_name, _iter.key); \
    _ret;                                                                                        \
  }

// checks that a string is the proper type
// params:
// - const jsmn_iterator _iter
// assumes the following variables exist:
// - const char* file_name
// - const char* mod_name
// - int error
#define END_JSON_CHECK_STRING(_iter) END_JSON_CHECK_STRING_RET(_iter, error++; continue);

// checks that string length is: min <= length <= max. custom return
// params:
// - const jsmn_iterator _iter
// assumes the following variables exist:
// - const char* file_name
// - const char* mod_name
#define END_JSON_CHECK_STRING_LENGTH_RET(_iter, _ret, min, max)                                   \
  {                                                                                               \
    int len = _iter.val->end - _iter.val->start;                                                  \
    if (len < min || len > max) {                                                                 \
      log_error("In data type %s from mod %s, %s is wrong size", file_name, mod_name, _iter.key); \
      _ret;                                                                                       \
    }                                                                                             \
  }

// checks that string length is: min <= length <= max
// params:
// - const jsmn_iterator _iter
// assumes the following variables exist:
// - const char* file_name
// - const char* mod_name
// - int error
#define END_JSON_CHECK_STRING_LENGTH(_iter, min, max) END_JSON_CHECK_STRING_LENGTH_RET(_iter, error++; continue, min, max);

// checks that an array is the proper type. custom return
// params:
// - const jsmn_iterator _iter
// assumes the following variables exist:
// - const char* file_name
// - const char* mod_name
#define END_JSON_CHECK_ARRAY_RET(_iter, _ret)                                                   \
  if (_iter.val->type != JSMN_ARRAY) {                                                          \
    log_error("In data type %s from mod %s, %s must be array", file_name, mod_name, _iter.key); \
    _ret;                                                                                       \
  }

// checks that an array is the proper type
// params:
// - const jsmn_iterator _iter
// assumes the following variables exist:
// - const char* file_name
// - const char* mod_name
// - int error
#define END_JSON_CHECK_ARRAY(_iter) END_JSON_CHECK_ARRAY_RET(_iter, error++; continue);

// checks that an array is the proper type and has at least one child. custom
// return
// params:
// - const jsmn_iterator _iter
// assumes the following variables exist:
// - const char* file_name
// - const char* mod_name
#define END_JSON_CHECK_ARRAY_AND_CHILDREN_RET(_iter, _ret)                                        \
  END_JSON_CHECK_ARRAY_RET(_iter, _ret);                                                          \
  if (_iter.val->size == 0) {                                                                     \
    log_error("In data type %s from mod %s, %s has no children", file_name, mod_name, _iter.key); \
    _ret;                                                                                         \
  }

// checks that an array is the proper type and has at least one child
// params:
// - const jsmn_iterator _iter
// assumes the following variables exist:
// - const char* file_name
// - const char* mod_name
// - int error
#define END_JSON_CHECK_ARRAY_AND_CHILDREN(_iter) END_JSON_CHECK_ARRAY_AND_CHILDREN_RET(_iter, error++; continue);

// checks that an object is the proper type. custom return
// params:
// - const jsmn_iterator _iter
// assumes the following variables exist:
// - const char* file_name
// - const char* mod_name
#define END_JSON_CHECK_OBJECT_RET(_iter, _ret)                                                   \
  if (_iter.val->type != JSMN_OBJECT) {                                                          \
    log_error("In data type %s from mod %s, %s must be object", file_name, mod_name, _iter.key); \
    _ret;                                                                                        \
  }

// checks that an object is the proper type
// params:
// - const jsmn_iterator _iter
// assumes the following variables exist:
// - const char* file_name
// - const char* mod_name
// - int error
#define END_JSON_CHECK_OBJECT(_iter) END_JSON_CHECK_OBJECT_RET(_iter, error++; continue);

// checks that an object is the proper type and has at least one child. custom
// return
// params:
// - const jsmn_iterator _iter
// assumes the following variables exist:
// - const char* file_name
// - const char* mod_name
#define END_JSON_CHECK_OBJECT_AND_CHILDREN_RET(_iter, _ret)                                       \
  END_JSON_CHECK_OBJECT_RET(_iter, _ret);                                                         \
  if (_iter.val->size == 0) {                                                                     \
    log_error("In data type %s from mod %s, %s has no children", file_name, mod_name, _iter.key); \
    _ret;                                                                                         \
  }

// checks that an object is the proper type and has at least one child
// params:
// - const jsmn_iterator _iter
// assumes the following variables exist:
// - const char* file_name
// - const char* mod_name
// - int error
#define END_JSON_CHECK_OBJECT_AND_CHILDREN(_iter) END_JSON_CHECK_OBJECT_AND_CHILDREN_RET(_iter, error++; continue);

#endif
