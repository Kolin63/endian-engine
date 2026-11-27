#include "discord_format.h"

#include <string.h>

#include <log.h>
#include <strint.h>

static bool
dfmt_time_type_validate(enum dfmt_time_type x) {
  switch (x) {
  case DFMT_LONG_DATETIME:
    return true;
  case DFMT_SHORT_DATETIME:
    return true;
  case DFMT_LONG_DATE:
    return true;
  case DFMT_SHORT_DATE:
    return true;
  case DFMT_SHORT_TIME:
    return true;
  case DFMT_LONG_TIME:
    return true;
  case DFMT_RELATIVE_TIME:
    return true;
  case DFMT_SHORT_DATETIME_SLASH:
    return true;
  case DFMT_LONG_DATETIME_SLASH:
    return true;
  default:
    return false;
  }
}

char*
dfmt_time(char buf[DFMT_TIME_BUF_LEN], enum dfmt_time_type fmt_type, time_t time) {
  if (!dfmt_time_type_validate(fmt_type)) {
    log_error("Improper discord format time type %i\n", fmt_type);
    strcpy(buf, "ERROR");
    return buf;
  }

  char time_buf[STRINT_64];
  u64_to_str(time_buf, time);

  char postfix[4] = ":X>";
  postfix[1] = (char)fmt_type;

  strcpy(buf, "<t:");
  strcat(buf, time_buf);
  strcat(buf, postfix);

  return buf;
}
