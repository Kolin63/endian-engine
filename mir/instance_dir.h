#ifndef ENDIAN_INSTANCE_DIR_H_
#define ENDIAN_INSTANCE_DIR_H_

int instance_dir_write(const char* instance_dir);

// caller is responsible for freeing returned string
// if first char is a /, absolute path
// else, relative to default root
char* instance_dir_expand();

#endif
