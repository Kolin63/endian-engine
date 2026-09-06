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
#include "../src/fileio.h"

ENDVEC_DEFINE(reflection_outs, struct reflection_out, reflection_out_cleanup(arr->arr + i));

char*
strcaps(const char* in) {
  size_t len = strlen(in);
  char* buf = malloc(len + 1);
  for (size_t i = 0; i < len; i++) {
    char c = in[i];
    if (c >= 'a' && c <= 'z') {
      buf[i] = c - ('a' - 'A');
    } else {
      buf[i] = c;
    }
  }
  buf[len] = '\0';
  return buf;
}

void
reflection_out_cleanup(struct reflection_out* ref) {
  if (ref == NULL) return;
  mirror_strings_cleanup(&ref->buf);
}

int
reflection_foreach_perfile_gen(struct mirror_strings* buf, const char* tag, const struct serial_file* sf,
                               const struct mirror_format_token* token) {
  int error = 0;

  struct serial_file_tag* sf_tag_block = NULL;
  if (tag != NULL) {
    for (size_t i = 0; i < sf->tags.len; i++) {
      if (strcmp(tag, sf->tags.arr[i].id) == 0) {
        sf_tag_block = sf->tags.arr + i;
        break;
      }
    }
    if (sf_tag_block == NULL) {
      return error;
    }
  }

  switch (token->type) {
  case MIR_CONST:
    mirror_strings_append(buf, strdup(token->buf));
    break;

  case MIR_FOREACH_BEGIN:
    log_error(MOD_STACK_FMT "unexpected MIR_FOREACH_BEGIN in already begun foreach", MOD_STACK_ARG);
    error++;
    break;

  case MIR_TAG_CONTENT:
    if (sf_tag_block == NULL) {
      log_error(MOD_STACK_FMT "mirror requesting tag content outside of a foreach block", MOD_STACK_ARG);
      error++;
      break;
    }
    mirror_strings_append(buf, strdup(sf_tag_block->buf));
    break;

  case MIR_DATA:
    if (sf_tag_block == NULL) {
      log_error(MOD_STACK_FMT "mirror requesting tag data outside of a foreach block", MOD_STACK_ARG);
      error++;
      break;
    }
    mirror_strings_append(buf, strdup(sf_tag_block->data));
    break;

  case MIR_DATA_CAPS:
    if (sf_tag_block == NULL) {
      log_error(MOD_STACK_FMT "mirror requesting tag data outside of a foreach block", MOD_STACK_ARG);
      error++;
      break;
    }
    mirror_strings_append(buf, strcaps(sf_tag_block->data));
    break;

  case MIR_NS:
    mirror_strings_append(buf, strdup(mod_stack_global()->ns));
    break;

  case MIR_NS_CAPS:
    mirror_strings_append(buf, strcaps(mod_stack_global()->ns));
    break;

  case MIR_ALPHA_SWITCH:
    mirror_strings_append(buf, strdup("// TODO"));
    break;
  }

  return error;
}

int
reflection_foreach_gen(struct mirror_strings* bufs, const char* tag, const struct serial_files* sf,
                       const struct mirror_format_token* token) {
  int error = 0;

  for (size_t i = 0; i < sf->len; i++) {
    error += reflection_foreach_perfile_gen(bufs + i, tag, sf->arr + i, token);
  }

  return error;
}

int
reflection_out_gen(struct reflection_out* ref, const struct serial_files* sf) {
  int error = 0;

  char* foreach_tag = NULL;
  bool in_foreach = false;

  struct mirror_strings* foreach_bufs = malloc(sf->len * sizeof(struct mirror_strings));
  for (size_t i = 0; i < sf->len; i++) {
    foreach_bufs[i].arr = NULL;
    foreach_bufs[i].len = 0;
    foreach_bufs[i].cap = 0;
  }

  const struct mirror_format_tokens* tokens = &ref->mir_file->tokens;
  for (size_t token_i = 0; token_i < tokens->len; token_i++) {
    const struct mirror_format_token* token = tokens->arr + token_i;

    if (in_foreach == true) {
      if (token->type != MIR_FOREACH_END) {
        error += reflection_foreach_gen(foreach_bufs, foreach_tag, sf, token);
      } else {
        in_foreach = false;
        for (size_t sf_i = 0; sf_i < sf->len; sf_i++) {
          for (size_t str_i = 0; str_i < foreach_bufs[sf_i].len; str_i++) {
            mirror_strings_append(&ref->buf, strdup(foreach_bufs[sf_i].arr[str_i]));
          }
          mirror_strings_cleanup(foreach_bufs + sf_i);
          foreach_bufs[sf_i].arr = NULL;
          foreach_bufs[sf_i].len = 0;
          foreach_bufs[sf_i].cap = 0;
        }
      }

      continue;
    }

    switch (token->type) {
    case MIR_CONST:
      mirror_strings_append(&ref->buf, strdup(token->buf));
      break;

    case MIR_FOREACH_BEGIN:
      foreach_tag = token->buf;
      in_foreach = true;
      break;
    }
  }

  mirror_strings_remove_backslashes(&ref->buf);

  free(foreach_bufs);

  return error;
}

int
reflection_out_write(const struct reflection_out* ref) {
  int error = 0;

  char* path = malloc(strlen(END_REF_SRC_DIR "/ref/") + strlen(ref->mir_file->name) + 1);
  strcpy(path, END_REF_SRC_DIR "/ref/");
  strcat(path, ref->mir_file->name);

  FILE* file = fopen(path, "w");

  free(path);

  if (file == NULL) {
    log_error(MOD_STACK_FMT "Could not open file for writing", MOD_STACK_ARG);
    error++;
    return error;
  }

  error += mirror_strings_write(file, &ref->buf);

  fclose(file);

  log_info("Generating file %s", ref->mir_file->name);

  return error;
}

int
reflection_outs_gen(struct reflection_outs* ref, const struct serial_files* sf) {
  int error = 0;

  for (size_t i = 0; i < ref->len; i++) {
    error += reflection_out_gen(ref->arr + i, sf);
  }

  error += reflection_outs_write(ref);

  return error;
}

int
reflection_outs_write(const struct reflection_outs* ref) {
  int error = 0;

  for (size_t i = 0; i < ref->len; i++) {
    error += reflection_out_write(ref->arr + i);
  }

  return error;
}

void
reflection_cleanup(struct reflection* ref) {
  if (ref == NULL) return;
  reflection_outs_cleanup(&ref->files);
}

int
reflection_gen(struct reflection* ref, const struct serial_files* sf) {
  int error = 0;
  error += reflection_outs_gen(&ref->files, sf);
  return error;
}

void
reflection_init(struct reflection* ref, const struct mirror* mir) {
  ref->files.len = mir->files.len;
  ref->files.cap = mir->files.cap;
  ref->files.arr = malloc(ref->files.cap * sizeof(struct reflection_out));
  for (size_t i = 0; i < ref->files.len; i++) {
    ref->files.arr[i].mir_file = mir->files.arr + i;
    ref->files.arr[i].buf.arr = NULL;
    ref->files.arr[i].buf.len = 0;
    ref->files.arr[i].buf.cap = 0;
  }
}
