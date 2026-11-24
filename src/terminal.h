#ifndef ENDIAN_TERMINAL_H_
#define ENDIAN_TERMINAL_H_

#include <stddef.h>
#include <pthread.h>

struct terminal_client {
  int sock;
  pthread_t thread;
  struct terminal_client* prev;
  struct terminal_client* next;
};

struct terminal {
  int sock;
  pthread_t thread;
  const char* port;
  struct terminal_client* clients;
};

void terminal_init(struct terminal* term, const char* port);
void terminal_start(struct terminal* term);
void terminal_cleanup(struct terminal* term);

// returns global sudo terminal
struct terminal* sudo_terminal_get();

#endif
