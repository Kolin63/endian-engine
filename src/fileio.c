#include "fileio.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define JSMN_HEADER
#include <concord/jsmn.h>
#include <log.h>

#define LINE_BUF_SIZE 65536

char* fileio_read_all(FILE* file) {
  char* buf = malloc(1);
  buf[0] = '\0';
  size_t buf_size = 1;
  char* line = malloc(LINE_BUF_SIZE);
  while (fgets(line, LINE_BUF_SIZE, file) != NULL) {
    buf_size += strlen(line);
    buf = realloc(buf, buf_size);
    strcat(buf, line);
  }
  free(line);
  return buf;
}

jsmntok_t* fileio_read_json(const char* json) {
  size_t json_len = strlen(json);

  jsmn_parser parser;
  size_t len = 128;

  jsmntok_t* tokens = NULL;

  while (true) {
    jsmn_init(&parser);
    tokens = realloc(tokens, len * sizeof(jsmntok_t));
    int err = jsmn_parse(&parser, json, json_len, tokens, len);

    if (err >= 0) {
      return tokens;
    } else if (err == JSMN_ERROR_NOMEM) {
      len += 128;
      continue;
    } else {
      if (err == JSMN_ERROR_INVAL) log_error("JSMN_ERROR_INVAL");
      else if (err == JSMN_ERROR_PART) log_error("JSMN_ERROR_PART");
      free(tokens);
      return NULL;
    }
  }
}
