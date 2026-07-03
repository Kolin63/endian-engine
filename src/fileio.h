#ifndef ENDIAN_FILEIO_H_
#define ENDIAN_FILEIO_H_

#include <stdio.h>

#define JSMN_HEADER
#include <concord/jsmn.h>

// Moves all contents of file into a buffer. buf must be freed after use
char* fileio_read_all(FILE* file);

jsmntok_t* fileio_read_json(const char* json);

#endif
