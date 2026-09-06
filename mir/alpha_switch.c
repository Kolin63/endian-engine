#include "alpha_switch.h"

#include <stdlib.h>
#include <string.h>

#include <log.h>

#include "mirror_strings.h"

ENDVEC_DEFINE(alpha_switch_matches, size_t);
ENDVEC_DEFINE(alpha_switch_cases, struct alpha_switch_case, alpha_switch_case_cleanup(arr->arr + i));

void
alpha_switch_case_cleanup(struct alpha_switch_case* alpha) {
  if (alpha == NULL) return;
  if (alpha->val != NULL) free(alpha->val);
  mirror_strings_cleanup(&alpha->buf);
  alpha->val = NULL;
}

void
alpha_switch_cleanup(struct alpha_switch* alpha) {
  if (alpha == NULL) return;
  if (alpha->switch_on != NULL) free(alpha->switch_on);
  alpha_switch_cases_cleanup(&alpha->cases);
  mirror_strings_cleanup(&alpha->def);
  alpha->escapes_encountered = 0;
  alpha->switch_on = NULL;
  alpha->buf = NULL;
}

void
alpha_switch_fillout(struct alpha_switch* alpha, const char* buf) {
  int mod = alpha->escapes_encountered % 5;

  switch (mod) {
  case 0:
    break;
  case 1:
    if (alpha->switch_on != NULL) break;
    if (strcmp(buf, "\n") == 0) break;
    alpha->switch_on = strdup(buf);
    {
      size_t len = strlen(alpha->switch_on);
      if (alpha->switch_on[len - 1] == '\n') alpha->switch_on[len - 1] = '\0';
    }
    break;
  case 2:
    if (strcmp(buf, "\n") == 0) break;
    if (alpha->cases.len != (alpha->escapes_encountered + 3) / 5) {
      alpha_switch_cases_append(&alpha->cases, (struct alpha_switch_case){});
    }
    struct alpha_switch_case* c = &alpha->cases.arr[alpha->cases.len - 1];
    if (c->val == NULL) {
      c->val = strdup(buf);
    } else {
      c->val = realloc(c->val, strlen(c->val) + strlen(buf) + 1);
      strcat(c->val, buf);
    }
    break;
  case 3:
    mirror_strings_append(&alpha->cases.arr[alpha->cases.len - 1].buf, strdup(buf));
    break;
  case 4:
    if (alpha->escapes_encountered > 4) break;
    mirror_strings_append(&alpha->def, strdup(buf));
    break;
  }
}

void
alpha_switch_gen_copy_mirror_strings(struct mirror_strings* buf, const struct mirror_strings* src) {
  for (size_t i = 0; i < src->len; i++) {
    mirror_strings_append(buf, strdup(src->arr[i]));
  }
}

void
alpha_switch_gen_sub(struct mirror_strings* buf, const struct alpha_switch* alpha, const struct alpha_switch_matches* matches, size_t i) {
  char numstr[256];
  snprintf(numstr, sizeof(numstr), "%zu", i);

  mirror_strings_append(buf, malloc(8 + strlen(alpha->switch_on) + 1 + strlen(numstr) + 5 + 1));
  char* str = buf->arr[buf->len - 1];
  strcpy(str, "switch (");
  strcat(str, alpha->switch_on);
  strcat(str, "[");
  strcat(str, numstr);
  strcat(str, "]) {\n");

  for (char c = 32; c < 127; c++) {
    struct alpha_switch_matches new_matches = {};
    for (size_t match_i = 0; match_i < matches->len; match_i++) {
      if (alpha->cases.arr[matches->arr[match_i]].val[i] == c) {
        alpha_switch_matches_append(&new_matches, matches->arr[match_i]);
      }
    }
    if (new_matches.len == 0) continue;

    mirror_strings_append(buf, malloc(6 + 1 + 4 + 1));
    str = buf->arr[buf->len - 1];
    strcpy(str, "case '");
    strcat(str, (char[]){c, '\0'});
    strcat(str, "':\n\n");

    if (new_matches.len == 1) {
      alpha_switch_gen_copy_mirror_strings(buf, &alpha->cases.arr[new_matches.arr[0]].buf);
    } else {
      alpha_switch_gen_sub(buf, alpha, &new_matches, i + 1);
    }

    alpha_switch_matches_cleanup(&new_matches);
  }

  mirror_strings_append(buf, malloc(9 + 1));
  str = buf->arr[buf->len - 1];
  strcpy(str, "default:\n");

  alpha_switch_gen_copy_mirror_strings(buf, &alpha->def);

  mirror_strings_append(buf, malloc(3 + 1));
  str = buf->arr[buf->len - 1];
  strcpy(str, "}\n\n");
}

int
alpha_switch_gen(struct mirror_strings* buf, struct alpha_switch* alpha) {
  if (alpha->escapes_encountered == 0) return 0;

  if (alpha->escapes_encountered % 5 != 0) {
    log_error("Incomplete alpha switch");
    return 1;
  }

  if (alpha->switch_on == NULL) {
    return 0;
  }

  struct alpha_switch_matches matches = {};
  for (size_t i = 0; i < alpha->cases.len; i++) {
    alpha_switch_matches_append(&matches, i);
  }

  alpha_switch_gen_sub(buf, alpha, &matches, 0);

  alpha_switch_matches_cleanup(&matches);
}
