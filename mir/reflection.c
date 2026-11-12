#include "reflection.h"

#include <log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "mirror.h"
#include "serial_file.h"
#include "mod_stack.h"
#include "../src/fileio.h"
#include "alpha_switch.h"

static void reflection_out_cleanup(struct reflection_out* out);

ENDVEC_DEFINE(reflection_outs, struct reflection_out, reflection_out_cleanup(arr->arr + i));

static char*
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

static void
reflection_out_cleanup(struct reflection_out* out) {
  if (out == NULL) return;
  mirror_strings_cleanup(&out->buf);
}

struct foreach_line_data {
  bool in;
  struct mirror_strings lines;
  struct mirror_strings_arr buf;
};

static void
foreach_line_gather_lines(struct foreach_line_data* foreach_line, const struct serial_file_tag* sf_tag_block) {
  char* buf = strdup(sf_tag_block->buf);
  char* line_start = buf;
  for (char* ptr = buf; *line_start != '\0'; ptr++) {
    if (*ptr == '\n') {
      *ptr = '\0';
      mirror_strings_append(&foreach_line->lines, strdup(line_start));
      mirror_strings_arr_append(&foreach_line->buf, (struct mirror_strings){});
      ptr++;
      line_start = ptr;
    }
  }
  free(buf);
}

static char*
foreach_lines_gen(struct foreach_line_data* foreach_line) {
  struct mirror_strings out = {};
  for (size_t buf_i = 0; buf_i < foreach_line->buf.len; buf_i++) {
    struct mirror_strings* line = foreach_line->buf.arr + buf_i;

    for (size_t word_i = 0; word_i < line->len; word_i++) {
      char** word = line->arr + word_i;
      if (strcmp(*word, "%l") == 0) {
        free(*word);
        *word = strdup(foreach_line->lines.arr[buf_i]);
      }
    }

    mirror_strings_append(line, strdup("\n"));

    char* linecat = mirror_strings_cat(line);
    mirror_strings_append(&out, linecat);
  }

  char* outcat = mirror_strings_cat(&out);
  mirror_strings_cleanup(&out);
  return outcat;
}

static int
reflection_foreach_pertoken_gen(struct reflection_out* out,
                                const struct mirror_format_token* token,
                                const struct serial_file_tag* sf_tag_block,
                                struct foreach_line_data* foreach_line) {
  int error = 0;

  char* line = NULL;
  switch (token->type) {
  case MIR_CONST:
    line = strdup(token->buf);
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
    line = strdup(sf_tag_block->buf);
    break;

  case MIR_DATA:
    if (sf_tag_block == NULL) {
      log_error(MOD_STACK_FMT "mirror requesting tag data outside of a foreach block", MOD_STACK_ARG);
      error++;
      break;
    }
    line = strdup(sf_tag_block->data);
    break;

  case MIR_DATA_CAPS:
    if (sf_tag_block == NULL) {
      log_error(MOD_STACK_FMT "mirror requesting tag data outside of a foreach block", MOD_STACK_ARG);
      error++;
      break;
    }
    line = strcaps(sf_tag_block->data);
    break;

  case MIR_MOD:
    line = strdup(mod_stack_global()->mod);
    break;

  case MIR_MOD_CAPS:
    line = strcaps(mod_stack_global()->mod);
    break;

  case MIR_NS:
    line = strdup(mod_stack_global()->ns);
    break;

  case MIR_NS_CAPS:
    line = strcaps(mod_stack_global()->ns);
    break;

  case MIR_ALPHA_SWITCH:
    out->alpha.escapes_encountered++;
    break;

  case MIR_BLAME: {
    const char* mod = mod_stack_global()->mod;
    const char* ns = mod_stack_global()->ns;
    const char* file = mod_stack_global()->file;
    char* blame = malloc(strlen(mod) + 1 + strlen(ns) + 1 + strlen(file) + 1);
    strcpy(blame, mod);
    strcat(blame, ":");
    strcat(blame, ns);
    strcat(blame, ":");
    strcat(blame, file);
    line = blame;
    break;
  }

  case MIR_FOREACH_LINE_BEGIN:
    if (foreach_line->in == true) {
      log_error("Start of foreach line when already in a foreach line");
      error++;
      break;
    }
    foreach_line->in = true;
    foreach_line->lines = (struct mirror_strings){};
    foreach_line->buf = (struct mirror_strings_arr){};
    foreach_line_gather_lines(foreach_line, sf_tag_block);
    break;

  case MIR_FOREACH_LINE_END:
    if (foreach_line->in == false) {
      log_error("End of foreach line when not in a foreach line");
      error++;
      break;
    }
    line = foreach_lines_gen(foreach_line);
    foreach_line->in = false;
    mirror_strings_cleanup(&foreach_line->lines);
    mirror_strings_arr_cleanup(&foreach_line->buf);
    break;

  case MIR_FOREACH_LINE:
    if (foreach_line->in == false) {
      log_error("Use of foreach line when not in a foreach line");
      error++;
      break;
    }
    line = strdup("%l");
    break;

  default:
    log_error("unknown token type");
    error++;
    break;
  }

  if (line == NULL) return error;

  if (out->alpha.escapes_encountered > 0) {
    alpha_switch_fillout(&out->alpha, line);
    free(line);
  } else if (foreach_line->in == true) {
    for (size_t buf_i = 0; buf_i < foreach_line->buf.len; buf_i++) {
      struct mirror_strings* buf = foreach_line->buf.arr + buf_i;
      mirror_strings_append(buf, strdup(line));
    }
    free(line);
  } else {
    mirror_strings_append(&out->buf, line);
  }

  return error;
}

static int
reflection_foreach_perfile_gen(struct reflection_out* out,
                               const struct mirror_format_tokens* tokens,
                               const struct serial_file* sf) {
  int error = 0;

  const char* tag = tokens->arr[-1].buf;

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

  struct foreach_line_data foreach_line = {};

  for (size_t token_i = 0; token_i < tokens->len; token_i++) {
    const struct mirror_format_token* token = tokens->arr + token_i;
    reflection_foreach_pertoken_gen(out, token, sf_tag_block, &foreach_line);
  }

  return error;
}

static int
reflection_foreach_gen(struct reflection_out* out,
                       const struct mirror_format_tokens* tokens,
                       const struct serial_files* sf) {
  int error = 0;

  for (size_t i = 0; i < sf->len; i++) {
    mod_stack_global()->file = sf->arr[i].name;
    error += reflection_foreach_perfile_gen(out, tokens, sf->arr + i);
  }

  return error;
}

static int
reflection_out_gen(struct reflection_out* out, const struct serial_files* sf) {
  int error = 0;

  bool in_foreach = false;
  struct mirror_format_tokens foreach_block = {};

  const struct mirror_format_tokens* tokens = &out->mir_file->tokens;
  for (size_t token_i = 0; token_i < tokens->len; token_i++) {
    const struct mirror_format_token* token = tokens->arr + token_i;

    if (in_foreach == true) {
      if (token->type != MIR_FOREACH_END) continue;

      foreach_block.len = token - foreach_block.arr;
      error += reflection_foreach_gen(out, &foreach_block, sf);
      error += alpha_switch_gen(&out->buf, &out->alpha);

      alpha_switch_cleanup(&out->alpha);
      foreach_block.arr = NULL;
      foreach_block.len = 0;
      in_foreach = false;

      continue;
    }

    switch (token->type) {
    case MIR_CONST:
      mirror_strings_append(&out->buf, strdup(token->buf));
      break;

    case MIR_FOREACH_BEGIN:
      in_foreach = true;
      foreach_block.arr = (struct mirror_format_token*)(token + 1);
      break;
    }
  }

  mirror_strings_remove_backslashes(&out->buf);

  return error;
}

static int
reflection_out_write(const struct reflection_out* out) {
  int error = 0;

  char* path = malloc(strlen(END_REF_SRC_DIR "/ref/") + strlen(out->mir_file->name) + 1);
  strcpy(path, END_REF_SRC_DIR "/ref/");
  strcat(path, out->mir_file->name);

  FILE* file = fopen(path, "w");

  free(path);

  if (file == NULL) {
    log_error(MOD_STACK_FMT "Could not open file for writing", MOD_STACK_ARG);
    error++;
    return error;
  }

  error += mirror_strings_write(file, &out->buf);

  fclose(file);

  log_info("Generating file %s", out->mir_file->name);

  return error;
}

static int
reflection_outs_write(const struct reflection_outs* out) {
  int error = 0;

  for (size_t i = 0; i < out->len; i++) {
    error += reflection_out_write(out->arr + i);
  }

  return error;
}

static int
reflection_outs_gen(struct reflection_outs* out, const struct serial_files* sf) {
  int error = 0;

  for (size_t i = 0; i < out->len; i++) {
    error += reflection_out_gen(out->arr + i, sf);
  }

  error += reflection_outs_write(out);

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
    ref->files.arr[i].buf = (struct mirror_strings){};
    ref->files.arr[i].alpha = (struct alpha_switch){};
  }
}
