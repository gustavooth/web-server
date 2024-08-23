#include "server.h"
#include "core/logger.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdatomic.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <stdatomic.h>

#define PORT 8080
#define RESPONSE "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 47\n\n<html><body><h1>Hello, World!</h1></body></html>"

#define MAX_CONNECTIONS 10
#define MAX_EVENTS 20
#define BUFFER_SIZE 1024

struct server_state {
  atomic_bool running;
  atomic_bool force;

  i32 fd;
  struct sockaddr_in address;

  i32 epoll_fd;
  struct epoll_event event;
  struct epoll_event events[MAX_EVENTS];
};

static struct server_state state;
pthread_t thread_server;

void* server_thread(void*);
void cls();
void* worker(void* data);

b8 server_run() {
  REXTRACE("initializing server...");

  if ((state.fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    REXERROR("Falha na criação do socket: %s", strerror(errno));
    return false;
  }

  state.address.sin_family = AF_INET;
  state.address.sin_addr.s_addr = INADDR_ANY;
  state.address.sin_port = htons(PORT);

  if (bind(state.fd, (struct sockaddr *)&state.address, sizeof(state.address)) < 0) {
    REXERROR("Falha no bind: %s", strerror(errno));
    close(state.fd);
    return false;
  }

  if (listen(state.fd, MAX_CONNECTIONS) < 0) {
    REXERROR("Falha no listen: %s", strerror(errno));
    close(state.fd);
    return false;
  }

  state.epoll_fd = epoll_create1(0);
  if (state.epoll_fd == -1) {
    REXERROR("epoll_create1: %s", strerror(errno));
    return false;
  }
  
  state.event.events = EPOLLIN;
  state.event.data.fd = state.fd;
  if (epoll_ctl(state.epoll_fd, EPOLL_CTL_ADD, state.fd, &state.event) == -1) {
    REXERROR("epoll_ctl: %s", strerror(errno));
    close(state.epoll_fd);
    close(state.fd);
    return false;
  }

  pthread_create(&thread_server, NULL, server_thread, NULL);

  REXINFO("Servidor iniciado na porta %d\n", PORT);
  return true;
}

void server_close() {
  if (!atomic_load(&state.running))
    return;

  REXTRACE("server: closure requested");
  
  if (atomic_load(&state.force)) {
    pthread_cancel(thread_server);
    cls();
    return;
  }

  atomic_store(&state.running, false);
}

void* server_thread(void*) {
  atomic_store(&state.running, true);
  while (atomic_load(&state.running))
  {
    atomic_store(&state.force, true);
    int nfds = epoll_wait(state.epoll_fd, state.events, MAX_EVENTS, -1);
    if (nfds == -1) {
      REXWARN("epoll_wait: %s", strerror(errno));
      return false;
    }
    atomic_store(&state.force, false);

    for (i32 i = 0; i < nfds; i++)
    {
      if (state.events[i].data.fd == state.fd)
      {
        // new connection
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd = accept(state.fd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_fd == -1) {
          REXWARN("accept: %s", strerror(errno));
          return false;
        }

        state.event.events = EPOLLIN | EPOLLET; // Edge-triggered
        state.event.data.fd = client_fd;
        if (epoll_ctl(state.epoll_fd, EPOLL_CTL_ADD, client_fd, &state.event) == -1) {
          REXWARN("epoll_ctl: %s", strerror(errno));
          close(client_fd);
          return false;
        }

        char* client_ip  = inet_ntoa(client_addr.sin_addr);
        REXTRACE("New client connected: %s", client_ip);
      }else {
        pthread_t thread;

        int* client_fd = malloc(sizeof(int));
        (*client_fd) = state.events[i].data.fd;

        pthread_create(&thread, NULL, worker, client_fd);
      }
    }
  }

  cls();
  return NULL;
}

void cls() {
  REXINFO("Closing server...");

  close(state.epoll_fd);
  close(state.fd);
  memset(&state, 0, sizeof(state));
}

void* worker(void* data) {
  i32 client_fd = *((i32*)data);

  char buffer[BUFFER_SIZE] = {0};

  read(client_fd, buffer, BUFFER_SIZE);
  REXTRACE("Request received:\n%s\n", buffer);

  write(client_fd, RESPONSE, strlen(RESPONSE));
  REXTRACE("Reply sent\n\n");

  close(client_fd);

  free(data);
  return NULL;
}

