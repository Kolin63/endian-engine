#include "strint.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define STRINT_MIN_U8   0
#define STRINT_MAX_U8   255
#define STRINT_MIN_I8  -128
#define STRINT_MAX_I8   127

#define STRINT_MIN_U16  0
#define STRINT_MAX_U16  65535
#define STRINT_MIN_I16 -32768
#define STRINT_MAX_I16  32767

#define STRINT_MIN_U32  0
#define STRINT_MAX_U32  4294967295
#define STRINT_MIN_I32 -2147483648
#define STRINT_MAX_I32  2147483647

#define STRINT_MIN_U64  0
#define STRINT_MAX_U64  18446744073709551615ull
#define STRINT_MIN_I64 -9223372036854775808ull
#define STRINT_MAX_I64  9223372036854775807ll

static inline void
strint_err_set(int* err, int val) {
  if (err != NULL) *err = val;
}

static inline bool
is_num_char(char c) {
  return (c >= '0') && (c <= '9');
}

static inline int
get_mult(char c) {
  return -1 * (c == '-') + 1 * (c != '-');
}

static inline const char*
get_start_of_signed_string(const char* str) {
  return str + 1 * (str[0] == '-' || str[0] == '+');
}

static inline int
char_to_int(char c) {
  return c - '0';
}

static uint64_t
generic_str_to_unsigned(const char* str, int* err, const uint64_t max) {
  if (str == NULL) {
    strint_err_set(err, STRINT_NULL_STR);
    return 0;
  }

  strint_err_set(err, STRINT_OK);

  uint64_t x = 0;

  const size_t len = strlen(str);
  for (size_t i = 0; i < len; i++) {
    const char c = str[i];

    if (!is_num_char(c)) {
      strint_err_set(err, STRINT_INVALID_CHAR);
      return 0;
    }

    const uint64_t prev = x;
    x *= 10;
    x += char_to_int(c);

    if (x < prev || x > max) {
      strint_err_set(err, STRINT_INT_OVERFLOW);
      return 0;
    }
  }

  return x;
}

static int64_t
generic_str_to_signed(const char* _str, int* err, const int64_t min, const int64_t max) {
  if (_str == NULL) {
    strint_err_set(err, STRINT_NULL_STR);
    return 0;
  }

  strint_err_set(err, STRINT_OK);

  const int mult = get_mult(_str[0]);
  const char* str = get_start_of_signed_string(_str);

  int64_t x = 0;

  const size_t len = strlen(str);
  for (size_t i = 0; i < len; i++) {
    const char c = str[i];

    if (!is_num_char(c)) {
      strint_err_set(err, STRINT_INVALID_CHAR);
      return 0;
    }

    const int64_t prev = x;
    x *= 10;
    x += mult * char_to_int(c);

    if ((x != STRINT_MIN_I64 && llabs(x) < llabs(prev)) || x < min || x > max) {
      strint_err_set(err, STRINT_INT_OVERFLOW);
      return 0;
    }
  }

  return x;
}

uint8_t
str_to_u8(const char* str, int* err) {
  return generic_str_to_unsigned(str, err, STRINT_MAX_U8);
}

int8_t
str_to_i8(const char* str, int* err) {
  return generic_str_to_signed(str, err, STRINT_MIN_I8, STRINT_MAX_I8);
}

uint16_t
str_to_u16(const char* str, int* err) {
  return generic_str_to_unsigned(str, err, STRINT_MAX_U16);
}

int16_t
str_to_i16(const char* str, int* err) {
  return generic_str_to_signed(str, err, STRINT_MIN_I16, STRINT_MAX_I16);
}

uint32_t
str_to_u32(const char* str, int* err) {
  return generic_str_to_unsigned(str, err, STRINT_MAX_U32);
}

int32_t
str_to_i32(const char* str, int* err) {
  return generic_str_to_signed(str, err, STRINT_MIN_I32, STRINT_MAX_I32);
}

uint64_t
str_to_u64(const char* str, int* err) {
  return generic_str_to_unsigned(str, err, STRINT_MAX_U64);
}

int64_t
str_to_i64(const char* str, int* err) {
  return generic_str_to_signed(str, err, STRINT_MIN_I64, STRINT_MAX_I64);
}

static inline uint64_t
ten_pow(unsigned int exponent) {
  switch (exponent) {
  case 0:  return 1;
  case 1:  return 10;
  case 2:  return 100;
  case 3:  return 1000;
  case 4:  return 10000;
  case 5:  return 100000;
  case 6:  return 1000000;
  case 7:  return 10000000;
  case 8:  return 100000000;
  case 9:  return 1000000000;
  case 10: return 10000000000;
  case 11: return 100000000000;
  case 12: return 1000000000000;
  case 13: return 10000000000000;
  case 14: return 100000000000000;
  case 15: return 1000000000000000;
  case 16: return 10000000000000000;
  case 17: return 100000000000000000;
  case 18: return 1000000000000000000;
  case 19: return 10000000000000000000ULL;
  default: return 0;
  }
}

static inline char
digit_to_char(unsigned int digit) {
  assert(digit >= 0 && digit <= 9);
  return '0' + digit;
}

static char*
generic_unsigned_to_str(char* buf, uint64_t x, int* err, int first_exp) {
  if (buf == NULL) {
    strint_err_set(err, STRINT_NULL_BUF);
    return NULL;
  }

  strint_err_set(err, STRINT_OK);

  if (x == 0) {
    buf[0] = '0';
    buf[1] = '\0';
    return buf;
  }

  char* next_digit = buf;

  for (int exp = first_exp; exp >= 0; exp--) {
    if (next_digit == buf && x < ten_pow(exp)) continue;

    int digit = x / ten_pow(exp);
    assert(digit >= 0 && digit <= 9);

    *next_digit = digit_to_char(digit);
    next_digit++;

    x -= digit * ten_pow(exp);
  }

  *next_digit = '\0';

  return buf;
}

static char*
generic_signed_to_str(char* buf, int64_t x, int* err, int first_exp) {
  if (buf == NULL) {
    strint_err_set(err, STRINT_NULL_BUF);
    return NULL;
  }

  char* new_buf;
  if (x < 0) {
    buf[0] = '-';
    new_buf = buf + 1;
    x *= -1;
  } else {
    new_buf = buf;
  }

  generic_unsigned_to_str(new_buf, x, err, first_exp);
  return buf;
}

char*
u8_to_str(char* buf, uint8_t x, int* err) {
  return generic_unsigned_to_str(buf, x, err, 2);
}

char*
i8_to_str(char* buf, int8_t x, int* err) {
  return generic_signed_to_str(buf, x, err, 2);
}

char*
u16_to_str(char* buf, uint16_t x, int* err) {
  return generic_unsigned_to_str(buf, x, err, 4);
}

char*
i16_to_str(char* buf, int16_t x, int* err) {
  return generic_signed_to_str(buf, x, err, 4);
}

char*
u32_to_str(char* buf, uint32_t x, int* err) {
  return generic_unsigned_to_str(buf, x, err, 9);
}

char*
i32_to_str(char* buf, int32_t x, int* err) {
  return generic_signed_to_str(buf, x, err, 9);
}

char*
u64_to_str(char* buf, uint64_t x, int* err) {
  return generic_unsigned_to_str(buf, x, err, 19);
}

char*
i64_to_str(char* buf, int64_t x, int* err) {
  return generic_signed_to_str(buf, x, err, 19);
}
