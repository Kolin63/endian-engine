#ifndef END_DISCORD_FORMAT_H_
#define END_DISCORD_FORMAT_H_

#include <time.h>

// https://dpp.dev  (time_format)
// https://sesh.fyi/timestamp/
enum dfmt_time_type {
  DFMT_LONG_DATETIME = 'F',         // "Tuesday, April 20, 2021 at 4:20 PM" - Long Date/Time
  DFMT_SHORT_DATETIME = 'f',        // "April 20, 2021 at 4:20 PM" - Short Date/Time
  DFMT_LONG_DATE = 'D',             // "April 21, 2021" - Long Date
  DFMT_SHORT_DATE = 'd',            // "4/20/2021" - Short Date
  DFMT_SHORT_TIME = 't',            // "4:20 PM" - Short Time
  DFMT_LONG_TIME = 'T',             // "4:20:00 PM" - Long Time
  DFMT_RELATIVE_TIME = 'R',         // "2 months ago" - Relative Time
  DFMT_SHORT_DATETIME_SLASH = 's',  // "4/20/2021, 4:20 PM" - Short Date/Time with slashes
  DFMT_LONG_DATETIME_SLASH = 'S',   // "4/20/2021, 4:20:00 PM" - LONG Date/Time with slashes
};

#define DFMT_TIME_BUF_LEN 32

char* dfmt_time(char buf[DFMT_TIME_BUF_LEN], enum dfmt_time_type fmt_type, time_t time);

#endif
