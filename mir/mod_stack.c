#include "mod_stack.h"

static struct mod_stack mod_stack = {
    .mod = "",
    .ns = "",
    .file = "",
};

struct mod_stack*
mod_stack_global() { return &mod_stack; }
