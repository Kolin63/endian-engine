#include "serial_file.h"

#include <stdlib.h>
#include <string.h>

#include <log.h>

#include "mirror_strings.h"
#include "mod_stack.h"

#define LINE_BUF_SIZE 65536

void
serial_file_tag_cleanup(struct serial_file_tag* x) {
  if (x == NULL) return;
  if (x->id != NULL) free(x->id);
  if (x->data != NULL) free(x->data);
  if (x->buf != NULL) free(x->buf);
}

// replaces closing parenthesis with null terminator and returns pointer
// to the first character after the opening parenthesis
char*
serial_file_tag_get_param(char* line) {
  char* start = NULL;
  for (char* i = line; *i != '\0'; i++) {
    if (*i == '(') {
      start = i + 1;
    } else if (*i == ')') {
      *i = '\0';
    }
  }
  return start;
}

void
serial_file_tags_cleanup(struct serial_file_tags* x) {
  if (x == NULL || x->len == 0) return;
  for (size_t i = 0; i < x->len; i++) {
    serial_file_tag_cleanup(&x->arr[i]);
  }
  free(x->arr);
}

int
serial_file_tags_fillout(struct serial_file_tags* tags, FILE* file) {
  int error = 0;

  char* line = malloc(LINE_BUF_SIZE);

  while (1) {
    char* tag_id = NULL;
    char* data = NULL;
    char* buf;

    if (fgets(line, LINE_BUF_SIZE, file) == NULL) break;

    if (strncmp(line, "ENDIAN_MIRROR_TAG_START(", 24) == 0) {
      char* str = serial_file_tag_get_param(line);
      tag_id = malloc(strlen(str) + 1);
      strcpy(tag_id, str);
      if (tag_id[0] == '\0') log_warn(MOD_STACK_FMT "mirror tag has no id", MOD_STACK_ARG);
    } else {
      if (tag_id != NULL) free(tag_id);
      continue;
    }

    buf = malloc(1);
    size_t buf_size = 1;
    buf[0] = '\0';

    while (1) {
      if (fgets(line, LINE_BUF_SIZE, file) == NULL) {
        log_warn(MOD_STACK_FMT "end of file before end of tag", MOD_STACK_ARG);
        break;
      } else if (strncmp(line, "ENDIAN_MIRROR_TAG_END()", 23) == 0) {
        break;
      } else if (strncmp(line, "ENDIAN_MIRROR_TAG_DATA(", 23) == 0) {
        const char* str = serial_file_tag_get_param(line);
        if (data != NULL) {
          log_warn(MOD_STACK_FMT "overriding previous tag data", MOD_STACK_ARG);
          free(data);
        }
        if (str[0] == '\0') log_warn(MOD_STACK_FMT "empty tag data", MOD_STACK_ARG);
        data = malloc(strlen(str) + 1);
        strcpy(data, str);
      } else {
        buf_size += strlen(line);
        buf = realloc(buf, buf_size);
        strcat(buf, line);
      }
    }

    if (data == NULL) data = "";

    tags->len++;
    tags->arr = realloc(tags->arr, tags->len * sizeof(struct serial_file_tag));

    struct serial_file_tag* tag = &tags->arr[tags->len - 1];

    tag->id = tag_id;
    tag->data = data;
    tag->buf = buf;
  }
  free(line);

  return error;
}

void
serial_file_cleanup(struct serial_file* x) {
  if (x == NULL) return;
  serial_file_tags_cleanup(&x->tags);
}

void
serial_file_load(struct serial_file* s, const char* file_path) {
  if (strncmp(mod_stack_global()->file, "template.", 9) == 0) return;

  s->tags.arr = NULL;
  s->tags.len = 0;

  FILE* file = fopen(file_path, "r");

  if (!file) {
    log_error(MOD_STACK_FMT "Could not open file (%s)", MOD_STACK_ARG, file_path);
    return;
  }

  if (serial_file_tags_fillout(&s->tags, file) != 0) {
    log_error(MOD_STACK_FMT "could not parse file", MOD_STACK_ARG);
    return;
  }

  fclose(file);

  log_info("Serializing file %s", mod_stack_global()->file);
}

void
serial_files_cleanup(struct serial_files* x) {
  if (x == NULL || x->len == 0) return;
  for (size_t i = 0; i < x->len; i++) {
    serial_file_cleanup(&x->arr[i]);
  }
  free(x->arr);
}
