#include "reflection.h"

#include <log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "mirror.h"
#include "serial_file.h"
#include "strings.h"
#include "mod_stack.h"

void
reflection_group_cleanup(struct reflection_group* ref) {
  if (ref == NULL) return;
  mirror_strings_cleanup(&ref->foreach_buf);
}

void
strcaps(char* buf, const char* in) {
  size_t len = strlen(in);
  for (size_t i = 0; i < len; i++) {
    char c = in[i];
    if (c >= 'a' && c <= 'z') {
      buf[i] = c - ('a' - 'A');
    } else {
      buf[i] = c;
    }
  }
  buf[len] = '\0';
}

int
reflection_group_gen(struct reflection_group* ref, const struct serial_file* sf) {
  int error = 0;

  const char* ns = mod_stack_global()->ns;
  char* ns_caps = malloc(strlen(ns) + 1);
  strcaps(ns_caps, ns);

  struct mirror_strings* buf = &ref->foreach_buf;

  for (size_t i = 0; i < ref->mir->foreach.len; i++) {
    const char* tag = ref->mir->foreach.arr[i].tag;
    const struct mirror_format_blocks* format = &ref->mir->foreach.arr[i].format;

    for (size_t j = 0; j < sf->tags.len; j++) {
      const struct serial_file_tag* sftag = sf->tags.arr + j;
      if (strcmp(sftag->id, tag) != 0) continue;

      const char* sftag_data;
      char* sftag_data_caps;

      if (sftag->data == NULL) {
        sftag_data = "";
        sftag_data_caps = "";
      } else {
        sftag_data = sftag->data;
        sftag_data_caps = malloc(strlen(sftag_data) + 1);
        strcaps(sftag_data_caps, sftag->data);
      }

      for (size_t k = 0; k < format->len; k++) {
        const struct mirror_format_block* block = format->arr + k;

        switch (block->type) {
        case MFBT_NULL:
          log_warn(MOD_STACK_FMT "Unexpected block type MFBT_NULL", MOD_STACK_ARG);
          break;
        case MFBT_CONST:
          buf->len += block->buf.len;
          buf->arr = realloc(buf->arr, buf->len * sizeof(char*));
          for (size_t l = buf->len - block->buf.len; l < buf->len; l++) {
            if (block->buf.arr[l + block->buf.len - buf->len] == NULL) {
              buf->arr[l] = NULL;
              continue;
            }
            buf->arr[l] = malloc(strlen(block->buf.arr[l + block->buf.len - buf->len]) + 1);
            strcpy(buf->arr[l], block->buf.arr[l + block->buf.len - buf->len]);
          }
          break;
        case MFBT_TAG_CONTENT:
          buf->len++;
          buf->arr = realloc(buf->arr, buf->len * sizeof(char*));
          buf->arr[buf->len - 1] = malloc(strlen(sftag->buf) + 1);
          strcpy(buf->arr[buf->len - 1], sftag->buf);
          break;
        case MFBT_DATA:
          buf->len++;
          buf->arr = realloc(buf->arr, buf->len * sizeof(char*));
          buf->arr[buf->len - 1] = malloc(strlen(sftag_data) + 1);
          strcpy(buf->arr[buf->len - 1], sftag_data);
          break;
        case MFBT_DATA_CAPS:
          buf->len++;
          buf->arr = realloc(buf->arr, buf->len * sizeof(char*));
          buf->arr[buf->len - 1] = malloc(strlen(sftag_data_caps) + 1);
          strcpy(buf->arr[buf->len - 1], sftag_data_caps);
          break;
        case MFBT_NS:
          buf->len++;
          buf->arr = realloc(buf->arr, buf->len * sizeof(char*));
          buf->arr[buf->len - 1] = malloc(strlen(ns) + 1);
          strcpy(buf->arr[buf->len - 1], ns);
          break;
        case MFBT_NS_CAPS:
          buf->len++;
          buf->arr = realloc(buf->arr, buf->len * sizeof(char*));
          buf->arr[buf->len - 1] = malloc(strlen(ns_caps) + 1);
          strcpy(buf->arr[buf->len - 1], ns_caps);
          break;
        case MFBT_ALPHA_SWITCH:
          buf->len++;
          buf->arr = realloc(buf->arr, buf->len * sizeof(char*));
          buf->arr[buf->len - 1] = malloc(strlen("// TODO") + 1);
          strcpy(buf->arr[buf->len - 1], "// TODO");
          break;
        }
      }
      if (sftag->data != NULL) free(sftag_data_caps);
    }
  }

  free(ns_caps);

  return error;
}

int
reflection_group_write(FILE* file, const struct reflection_group* ref) {
  int error = 0;

  error += mirror_strings_write(file, &ref->mir->prefix);
  error += mirror_strings_write(file, &ref->foreach_buf);
  error += mirror_strings_write(file, &ref->mir->postfix);

  return error;
}

void
reflection_groups_cleanup(struct reflection_groups* ref) {
  if (ref == NULL || ref->len == 0 || ref->arr == NULL) return;
  for (size_t i = 0; i < ref->len; i++) {
    reflection_group_cleanup(ref->arr + i);
  }
  free(ref->arr);
}

int
reflection_groups_write(FILE* file, const struct reflection_groups* ref) {
  int error = 0;

  for (size_t i = 0; i < ref->len; i++) {
    error += reflection_group_write(file, ref->arr + i);
  }

  return error;
}

int
reflection_groups_gen(struct reflection_groups* ref, const struct serial_file* sf) {
  int error = 0;

  for (size_t i = 0; i < ref->len; i++) {
    error += reflection_group_gen(ref->arr + i, sf);
  }

  return error;
}

void
reflection_file_cleanup(struct reflection_file* ref) {
  if (ref == NULL) return;
  reflection_groups_cleanup(&ref->groups);
}

int
reflection_file_gen(struct reflection_file* ref, const struct serial_file* sf) {
  int error = 0;

  error += reflection_groups_gen(&ref->groups, sf);

  return error;
}

int
reflection_file_write(const struct reflection_file* ref) {
  int error = 0;

  char* path = malloc(4 + strlen(ref->name) + 1);
  strcpy(path, "ref/");
  strcat(path, ref->name);

  FILE* file = fopen(path, "w");

  free(path);

  if (file == NULL) {
    log_error(MOD_STACK_FMT "Could not open file for writing", MOD_STACK_ARG);
    error++;
    return error;
  }

  error += reflection_groups_write(file, &ref->groups);

  fclose(file);

  return error;
}

void
reflection_files_cleanup(struct reflection_files* ref) {
  if (ref == NULL || ref->len == 0 || ref->arr == NULL) return;
  for (size_t i = 0; i < ref->len; i++) {
    reflection_file_cleanup(ref->arr + i);
  }
  free(ref->arr);
}

int
reflection_files_gen(struct reflection_files* ref, const struct serial_files* sf) {
  int error = 0;

  for (size_t i = 0; i < ref->len; i++) {
    for (size_t j = 0; j < sf->len; j++) {
      mod_stack_global()->file = sf->arr[j].name;
      error += reflection_file_gen(ref->arr + i, sf->arr + j);
    }
  }

  error += reflection_files_write(ref);

  return error;
}

int
reflection_files_write(const struct reflection_files* ref) {
  int error = 0;

  FILE* dir = fopen("ref", "r");
  if (dir == NULL) {
    mkdir("ref", 0b111111111);
    dir = fopen("ref", "r");
    if (dir == NULL) {
      log_error("Could not create ref/ directory");
      error++;
      return error;
    } else {
      fclose(dir);
    }
  } else {
    fclose(dir);
  }

  for (size_t i = 0; i < ref->len; i++) {
    error += reflection_file_write(ref->arr + i);
  }

  return error;
}

void
reflection_cleanup(struct reflection* ref) {
  if (ref == NULL) return;
  reflection_files_cleanup(&ref->files);
}

int
reflection_gen(struct reflection* ref, const struct serial_files* sf) {
  int error = 0;
  error += reflection_files_gen(&ref->files, sf);
  return error;
}

void
reflection_init(struct reflection* ref, const struct mirror* mir) {
  ref->files.len = mir->files.len;
  ref->files.arr = malloc(ref->files.len * sizeof(struct reflection_file));
  for (size_t i = 0; i < ref->files.len; i++) {
    ref->files.arr[i].name = mir->files.arr[i].name;
    ref->files.arr[i].groups.len = mir->files.arr[i].groups.len;
    ref->files.arr[i].groups.arr = malloc(ref->files.arr[i].groups.len * sizeof(struct reflection_group));
    for (size_t j = 0; j < ref->files.arr[i].groups.len; j++) {
      ref->files.arr[i].groups.arr[j].mir = mir->files.arr[i].groups.arr + j;
      ref->files.arr[i].groups.arr[j].foreach_buf.arr = NULL;
      ref->files.arr[i].groups.arr[j].foreach_buf.len = 0;
    }
  }
}
