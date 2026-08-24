#ifndef ENDIAN_FUNCTION_H_
#define ENDIAN_FUNCTION_H_

// this is the order in which the functions will be called
void function_call_init();
void function_call_load();
void function_call_save();
void function_call_cleanup();

#include "../ref/functions.h"

#endif
