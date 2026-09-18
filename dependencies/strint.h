#ifndef MELICAN_STRINT_H_
#define MELICAN_STRINT_H_

#include <stdint.h>

enum {
  STRINT_OK = 0,
  STRINT_INVALID_CHAR = 1,
  STRINT_INT_OVERFLOW = 2,
  STRINT_NULL_STR = 3,
  STRINT_NULL_BUF = 4,
};

// In string to integer functions, an invalid character will result in 0 being
// returned and err being set to STRINT_INVALID_CHAR.
// If the integer is overflowed, 0 will be returned and err will be set to
// STRINT_INT_OVERFLOW.

uint8_t  str_to_u8(const char* str, int* err);
int8_t   str_to_i8(const char* str, int* err);
uint16_t str_to_u16(const char* str, int* err);
int16_t  str_to_i16(const char* str, int* err);
uint32_t str_to_u32(const char* str, int* err);
int32_t  str_to_i32(const char* str, int* err);
uint64_t str_to_u64(const char* str, int* err);
int64_t  str_to_i64(const char* str, int* err);

// In integer to string functions, the buffer that is passed in is assumed to be
// of the sizes defined as STRINT_8, STRINT_16, STRINT_32, or STRINT_64.
// A pointer to the buffer is returned as a convenience to the caller.

#define STRINT_8  8
#define STRINT_16 8
#define STRINT_32 16
#define STRINT_64 32

char* u8_to_str(char* buf, uint8_t x, int* err);
char* i8_to_str(char* buf, int8_t x, int* err);
char* u16_to_str(char* buf, uint16_t x, int* err);
char* i16_to_str(char* buf, int16_t x, int* err);
char* u32_to_str(char* buf, uint32_t x, int* err);
char* i32_to_str(char* buf, int32_t x, int* err);
char* u64_to_str(char* buf, uint64_t x, int* err);
char* i64_to_str(char* buf, int64_t x, int* err);

#endif
