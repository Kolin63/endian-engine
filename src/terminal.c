#include "terminal.h"

#include "exit.h"

#include <log.h>

#include <pthread.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#define END_TERM_BUF_SIZE 1024
#define MESSAGE "test foo bar"

#define O_NONBLOCK

static struct terminal global_sudo_terminal;

static struct terminal_client*
terminal_add_client(struct terminal* term, const struct terminal_client* client) {
  log_debug("adding client");

  struct terminal_client* copy = malloc(sizeof(struct terminal_client));
  copy->sock = client->sock;
  copy->thread = client->thread;

  copy->prev = NULL;
  copy->next = term->clients;
  term->clients = copy;

  return copy;
}

static void
terminal_client_cleanup(struct terminal_client* client) {
  pthread_cancel(client->thread);
  shutdown(client->sock, SHUT_RDWR);
  close(client->sock);
  client->sock = -1;
  client->prev = (struct terminal_client*)0xdeadbeef;
  client->next = (struct terminal_client*)0xdeadbeef;
  free(client);
}

static void
terminal_remove_client(struct terminal_client* client) {
  struct terminal_client* prev = client->prev;
  struct terminal_client* next = client->next;
  if (prev != NULL) prev->next = next;
  if (next != NULL) next->prev = prev;
  terminal_client_cleanup(client);
}

static void
terminal_cleanup_clients(struct terminal* term) {
  struct terminal_client* i = term->clients;
  struct terminal_client* next = NULL;

  while (i != NULL) {
    next = i->next;
    terminal_client_cleanup(i);
    i = next;
  }
}

static struct addrinfo*
terminal_get_addrinfo(const char* port) {
  const struct addrinfo hints = {
    .ai_flags = AI_PASSIVE,
    .ai_family = AF_INET6,
    .ai_socktype = SOCK_STREAM,
    .ai_protocol = getprotobyname("tcp")->p_proto,
  };

  struct addrinfo* ai;

  int err = getaddrinfo(NULL, port, &hints, &ai);
  if (err != 0) {
    log_error("Could not get address info for terminal. '%s' (%i). errno: %i",
              gai_strerror(err), err, errno);
    return NULL;
  }

  return ai;
}

static int
terminal_get_sock(struct addrinfo* ai) {
  int sock;
  struct addrinfo* ai_ptr;
  for (ai_ptr = ai; ai_ptr != NULL; ai_ptr = ai_ptr->ai_next) {
    sock = socket(ai_ptr->ai_family, ai_ptr->ai_socktype, ai_ptr->ai_protocol);
    if (sock == -1) continue;

    if (bind(sock, ai_ptr->ai_addr, ai_ptr->ai_addrlen) == 0) break;  // success

    close(sock);
  }

  if (ai_ptr == NULL || sock == -1) {
    log_error("Could not bind terminal socket. errno: %i", errno);
    return -1;
  }

  return sock;
}

void
terminal_init(struct terminal* term, const char* port) {
  struct addrinfo* ai = terminal_get_addrinfo(port);

  term->sock = terminal_get_sock(ai);
  term->port = port;

  freeaddrinfo(ai);

  if (term->sock == -1) abort_cleanup(-1);
}

static void*
terminal_client_start_thread(struct terminal_client* client) {
  log_debug("accpeted client");

  while (true) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    char buf[END_TERM_BUF_SIZE];
    ssize_t readlen = read(client->sock, buf, END_TERM_BUF_SIZE);

    if (readlen == 0) {
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      continue;
    }

    if (readlen == -1) {
      log_error("Could not read from client. errno: %i", errno);
      continue;
    }

    log_debug("Received %zd bytes from client: '%s'", readlen, buf);

    write(client->sock, MESSAGE, sizeof(MESSAGE));
  }
}

static void*
terminal_start_thread(struct terminal* term) {
  log_info("Starting terminal on port %s", term->port);

  listen(term->sock, SOMAXCONN);

  while (true) {
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len;

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    int client_sock = accept(term->sock, (struct sockaddr*)&client_addr, &client_addr_len);

    if (client_sock == -1) {
      if (errno != EAGAIN && errno != EWOULDBLOCK) {
        log_error("Could not accept connection from client terminal. errno: %i", errno);
      }
      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      continue;
    }

    struct terminal_client* client = terminal_add_client(term, &(struct terminal_client){});
    client->sock = client_sock;
    pthread_create(&client->thread, NULL, (void*)terminal_client_start_thread, client);
  }
}

void
terminal_start(struct terminal* term) {
  pthread_create(&term->thread, NULL, (void*)terminal_start_thread, term);
}

void
terminal_cleanup(struct terminal* term) {
  log_info("Shutting down terminal...");

  // cancel thread and wait until it has been cancelled
  pthread_cancel(term->thread);
  void* retval;
  do {
    log_debug("attempting join");
    pthread_join(term->thread, &retval);
  } while (retval != PTHREAD_CANCELED);

  terminal_cleanup_clients(term);
  shutdown(term->sock, SHUT_RDWR);
  close(term->sock);
}

struct terminal*
sudo_terminal_get() {
  return &global_sudo_terminal;
}
