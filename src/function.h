#ifndef ENDIAN_FUNCTION_H_
#define ENDIAN_FUNCTION_H_

// this is the order in which the functions will be called
static void function_call_init();
static void function_call_load();
static void function_call_save();
static void function_call_cleanup();

#include "../ref/functions.h"

#endif
