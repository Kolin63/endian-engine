#include "serial_file.h"

#include <stdlib.h>
#include <string.h>

#include <log.h>

#include "mirror_strings.h"
#include "mod_stack.h"

#define LINE_BUF_SIZE 65536

ENDVEC_DEFINE(serial_file_tags, struct serial_file_tag, serial_file_tag_cleanup(arr->arr + i));
ENDVEC_DEFINE(serial_files, struct serial_file, serial_file_cleanup(arr->arr + i));

void
serial_file_tag_cleanup(struct serial_file_tag* x) {
  if (x == NULL) return;
  if (x->id != NULL) free(x->id);
  if (x->data != NULL) free(x->data);
  if (x->buf != NULL) free(x->buf);
}

// returns pointer to first character after space and removes trailing newline
char*
serial_file_tag_get_param(char* line) {
  char* start = line;
  while (true) {
    if (*start == '\0') return start;
    else if (*start == ' ') {
      start++;
      break;
    }
    start++;
  }
  while (*start == ' ') {
    start++;
  }

  char* end = start;
  while (*end != '\n' && *end != '\0') {
    end++;
  }
  *end = '\0';

  return start;
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

    if (strncmp(line, "#mir_tag_start", 14) == 0) {
      char* str = serial_file_tag_get_param(line);
      tag_id = malloc(strlen(str) + 1);
      strcpy(tag_id, str);
      if (tag_id[0] == '\0') log_warn(MOD_STACK_FMT "mirror tag has no id", MOD_STACK_ARG);
    } else {
      continue;
    }

    buf = malloc(1);
    size_t buf_size = 1;
    buf[0] = '\0';

    while (1) {
      if (fgets(line, LINE_BUF_SIZE, file) == NULL) {
        log_warn(MOD_STACK_FMT "end of file before end of tag", MOD_STACK_ARG);
        break;
      } else if (strncmp(line, "#mir_tag_end", 12) == 0) {
        break;
      } else if (strncmp(line, "#mir_tag_data", 13) == 0) {
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

    serial_file_tags_append(tags, (struct serial_file_tag){});

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
  if (x->name != NULL) free(x->name);
  serial_file_tags_cleanup(&x->tags);
}

void
serial_file_load(struct serial_file* s, const char* file_path, const char* file_name) {
  s->name = NULL;
  s->tags.arr = NULL;
  s->tags.len = 0;
  s->tags.cap = 0;

  if (strncmp(mod_stack_global()->file, "template.", 9) == 0) return;

  s->name = strdup(file_name);

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
