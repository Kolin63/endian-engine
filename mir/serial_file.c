#include "serial_file.h"

#include <stdlib.h>

#include "mirror_strings.h"

void serial_file_tag_cleanup(struct serial_file_tag* x) {
  if (x == NULL) return;
  if (x->id != NULL) free(x->id);
  mirror_strings_cleanup(&x->data);
  if (x->buf != NULL) free(x->buf);
}

void serial_file_tags_cleanup(struct serial_file_tags* x) {
  if (x == NULL) return;
  for (size_t i = 0; i < x->len; x++) {
    serial_file_tag_cleanup(&x->arr[i]);
  }
}

void serial_file_cleanup(struct serial_file* x) {
  if (x == NULL) return;
  serial_file_tags_cleanup(&x->tags);
}

void serial_files_cleanup(struct serial_files* x) {
  if (x == NULL) return;
  for (size_t i = 0; i < x->len; i++) {
    serial_file_cleanup(&x->arr[i]);
  }
}
