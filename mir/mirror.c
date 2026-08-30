#include "mirror.h"

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#define JSMN_HEADER
#include <log.h>
#include <concord/jsmn.h>

#include "jsmn_iterator.h"
#include "json_macros.h"
#include "mod_stack.h"
#include "../src/fileio.h"

static struct mirrors global = {};

ENDVEC_DEFINE(mirror_format_blocks, struct mirror_format_block, mirror_format_block_cleanup(arr->arr + i));
ENDVEC_DEFINE(mirror_foreach_arr, struct mirror_foreach, mirror_foreach_cleanup(arr->arr + i));
ENDVEC_DEFINE(mirror_groups, struct mirror_group, mirror_group_cleanup(arr->arr + i));
ENDVEC_DEFINE(mirror_files, struct mirror_file, mirror_file_cleanup(arr->arr + i));
ENDVEC_DEFINE(mirrors, struct mirror, mirror_cleanup(arr->arr + i));

void
mirror_format_block_cleanup(struct mirror_format_block* f) {
  if (f == NULL) return;
  mirror_strings_cleanup(&f->buf);
}

int
mirror_format_blocks_from_json(struct mirror_format_blocks* f, const jsmntok_t* jsmn, const char* json) {
  int error = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_ARRAY_RET(iter, error++; return error);

  f->len = 1;
  f->arr = malloc(sizeof(struct mirror_format_block));
  struct mirror_format_block* current_block = &f->arr[0];
  current_block->type = MFBT_NULL;
  current_block->buf.arr = NULL;
  current_block->buf.len = 0;

  while (jsmn_iterator_next(&iter)) {
    END_JSON_CHECK_STRING(iter);
    char* line = jsmn_iterator_get_string_heap(json, iter.val);
    const size_t linelen = strlen(line) + 1;
    line = realloc(line, linelen + 1);
    strcat(line, "\n");

    bool escape = false;

    for (size_t i = 0; i < linelen; i++) {
      if (line[i] == '%') {
        if (escape == true) {
          escape = false;
        } else {
          escape = true;
          continue;
        }
      }

      if (escape == true) {
        escape = false;
        enum mirror_format_block_type type;
        switch (line[i]) {
        case 't':
          type = MFBT_TAG_CONTENT;
          break;
        case 'd':
          type = MFBT_DATA;
          break;
        case 'D':
          type = MFBT_DATA_CAPS;
          break;
        case 'n':
          type = MFBT_NS;
          break;
        case 'N':
          type = MFBT_NS_CAPS;
          break;
        case 'A':
          type = MFBT_ALPHA_SWITCH;
          break;
        default:
          log_error(MOD_STACK_FMT "unknown escape sequence %%%c", MOD_STACK_ARG, line[i]);
          error++;
          continue;
        }
        if (current_block->type != MFBT_NULL) {
          mirror_format_blocks_append(f, (struct mirror_format_block){});
          current_block = &f->arr[f->len - 1];
        }
        current_block->type = type;
        current_block->buf.len = 0;
        current_block->buf.arr = NULL;

        continue;
      }

      // at this point we are sure the text is of type CONST

      if (current_block->type != MFBT_CONST) {
        if (current_block->type != MFBT_NULL) {
          mirror_format_blocks_append(f, (struct mirror_format_block){});
          current_block = &f->arr[f->len - 1];
        }
        current_block->type = MFBT_CONST;
        current_block->buf.len = 0;
        current_block->buf.arr = NULL;
      }

      if (current_block->buf.arr == NULL || current_block->buf.len == 0) {
        mirror_strings_append(&current_block->buf, NULL);
      }

      char* bufstr = current_block->buf.arr[current_block->buf.len - 1];

      if (bufstr == NULL) {
        current_block->buf.arr[current_block->buf.len - 1] = malloc(linelen + 1);
        current_block->buf.arr[current_block->buf.len - 1][0] = '\0';
        bufstr = current_block->buf.arr[current_block->buf.len - 1];
      }

      char charstr[2] = {line[i], '\0'};
      strcat(bufstr, charstr);
    }

    free(line);

    if (current_block->type == MFBT_CONST) {
      mirror_strings_append(&current_block->buf, NULL);
    }
  }

  return error;
}

void
mirror_foreach_cleanup(struct mirror_foreach* f) {
  free(f->tag);
  mirror_format_blocks_cleanup(&f->format);
}

int
mirror_foreach_from_json(struct mirror_foreach* f, const jsmntok_t* jsmn, const char* json) {
  int error = 0;

  f->tag = NULL;
  f->format.arr = NULL;
  f->format.len = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_OBJECT_RET(iter, error++; return error);

  while (jsmn_iterator_next(&iter)) {
    if (strcmp(iter.key, "tag") == 0) {
      END_JSON_CHECK_STRING(iter);
      f->tag = jsmn_iterator_get_string_heap(json, iter.val);
    } else if (strcmp(iter.key, "format") == 0) {
      END_JSON_CHECK_ARRAY(iter);
      int this_error = mirror_format_blocks_from_json(&f->format, iter.val, json);
      if (this_error != 0) {
        error += this_error;
      } else {
        for (size_t i = 0; i < f->format.len; i++) {
          mirror_strings_remove_backslashes(&f->format.arr[i].buf);
        }
      }
    } else {
      error++;
      log_error(MOD_STACK_FMT "Unknown object %s", MOD_STACK_ARG, iter.key);
    }
  }

  return error;
}

int
mirror_foreach_arr_from_json(struct mirror_foreach_arr* arr, const jsmntok_t* jsmn, const char* json) {
  int error = 0;

  arr->arr = NULL;
  arr->len = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_ARRAY_RET(iter, error++; return error);

  while (jsmn_iterator_next(&iter)) {
    mirror_foreach_arr_append(arr, (struct mirror_foreach){});
    error += mirror_foreach_from_json(&arr->arr[arr->len - 1], iter.val, json);
  }

  return error;
}

void
mirror_group_cleanup(struct mirror_group* g) {
  if (g == NULL) return;
  mirror_strings_cleanup(&g->prefix);
  mirror_foreach_arr_cleanup(&g->foreach);
  mirror_strings_cleanup(&g->postfix);
}

int
mirror_group_from_json(struct mirror_group* g, const jsmntok_t* jsmn, const char* json) {
  int error = 0;

  g->prefix.arr = NULL;
  g->prefix.len = 0;
  g->foreach.arr = NULL;
  g->foreach.len = 0;
  g->postfix.arr = NULL;
  g->postfix.len = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_OBJECT_RET(iter, error++; return error);

  while (jsmn_iterator_next(&iter)) {
    if (strcmp(iter.key, "prefix") == 0) {
      END_JSON_CHECK_ARRAY(iter);
      int this_error = mirror_strings_from_json(&g->prefix, iter.val, json);
      if (this_error != 0) {
        error += this_error;
      } else {
        mirror_strings_remove_backslashes(&g->prefix);
        mirror_strings_append_newline_to_all(&g->prefix);
      }
    } else if (strcmp(iter.key, "foreach") == 0) {
      END_JSON_CHECK_ARRAY(iter);
      error += mirror_foreach_arr_from_json(&g->foreach, iter.val, json);
    } else if (strcmp(iter.key, "postfix") == 0) {
      END_JSON_CHECK_ARRAY(iter);
      int this_error = mirror_strings_from_json(&g->postfix, iter.val, json);
      if (this_error != 0) {
        error += this_error;
      } else {
        mirror_strings_remove_backslashes(&g->postfix);
        mirror_strings_append_newline_to_all(&g->postfix);
      }
    } else {
      error++;
      log_error(MOD_STACK_FMT "Unknown object %s", MOD_STACK_ARG, iter.key);
    }
  }

  return error;
}

int
mirror_groups_from_json(struct mirror_groups* arr, const jsmntok_t* jsmn, const char* json) {
  int error = 0;

  arr->arr = NULL;
  arr->len = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_ARRAY_RET(iter, error++; return error);

  while (jsmn_iterator_next(&iter)) {
    mirror_groups_append(arr, (struct mirror_group){});
    error += mirror_group_from_json(&arr->arr[arr->len - 1], iter.val, json);
  }

  return error;
}

void
mirror_file_cleanup(struct mirror_file* f) {
  if (f == NULL) return;
  free(f->name);
  mirror_groups_cleanup(&f->groups);
}

int
mirror_file_from_json(struct mirror_file* f, const jsmntok_t* jsmn, const char* json) {
  int error = 0;

  f->name = NULL;
  f->groups.arr = NULL;
  f->groups.len = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_OBJECT_RET(iter, error++; return error);

  while (jsmn_iterator_next(&iter)) {
    if (strcmp(iter.key, "name") == 0) {
      END_JSON_CHECK_STRING(iter);
      f->name = jsmn_iterator_get_string_heap(json, iter.val);
    } else if (strcmp(iter.key, "groups") == 0) {
      END_JSON_CHECK_ARRAY(iter);
      error += mirror_groups_from_json(&f->groups, iter.val, json);
    } else {
      error++;
      log_error(MOD_STACK_FMT "Unknown object %s", MOD_STACK_ARG, iter.key);
    }
  }

  return error;
}

int
mirror_files_from_json(struct mirror_files* arr, const jsmntok_t* jsmn, const char* json) {
  int error = 0;

  arr->arr = NULL;
  arr->len = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_ARRAY_RET(iter, error++; return error);

  while (jsmn_iterator_next(&iter)) {
    mirror_files_append(arr, (struct mirror_file){});
    error += mirror_file_from_json(&arr->arr[arr->len - 1], iter.val, json);
  }

  return error;
}

void
mirror_cleanup(struct mirror* m) {
  if (m == NULL) return;
  if (m->id != NULL) free(m->id);
  mirror_files_cleanup(&m->files);
}

int
mirror_from_json(struct mirror* m, const jsmntok_t* jsmn, const char* json) {
  int error = 0;

  m->id = NULL;
  m->files.arr = NULL;
  m->files.len = 0;

  struct jsmn_iterator iter;
  jsmn_iterator_init(&iter, jsmn, json);

  END_JSON_CHECK_OBJECT_RET(iter, error++; return error);

  while (jsmn_iterator_next(&iter)) {
    if (strcmp(iter.key, "id") == 0) {
      END_JSON_CHECK_STRING(iter);
      m->id = jsmn_iterator_get_string_heap(json, iter.val);
    } else if (strcmp(iter.key, "files") == 0) {
      END_JSON_CHECK_ARRAY(iter);
      error += mirror_files_from_json(&m->files, iter.val, json);
    } else {
      error++;
      log_error(MOD_STACK_FMT "Unknown object %s", MOD_STACK_ARG, iter.key);
    }
  }

  return error;
}

void
mirror_load(const char* file_path) {
  if (strcmp(mod_stack_global()->file, "template.json") == 0) return;

  FILE* file = fopen(file_path, "r");

  if (!file) {
    log_error(MOD_STACK_FMT "Could not open mirror file (%s)", MOD_STACK_ARG, file_path);
    return;
  }

  char* json = fileio_read_all(file);
  fclose(file);

  mirror_load_from_str(json);
  free(json);
}

void
mirror_load_from_str(const char* str) {
  jsmntok_t* jsmn = fileio_read_json(str);

  struct mirror m;
  if (mirror_from_json(&m, jsmn, str) != 0) {
    free(jsmn);
    return;
  }

  free(jsmn);

  mirrors_append(&global, m);

  log_info("Loading mirror %s", m.id);
}

struct mirrors*
mirrors_global() {
  return &global;
}
