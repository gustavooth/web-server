#include "commands.h"

#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include "core/logger.h"
#include "string.h"
#include "server.h"

pthread_t thread_commands;
atomic_bool commands_running;

void* commands_thread(void*);
void close_command();

b8 server_commands_run() {
  pthread_create(&thread_commands, NULL, commands_thread, NULL);
  return true;
}

void server_commands_close() {
  close_command();
}

void* commands_thread(void*) {
  atomic_store(&commands_running, true);

  char command[100];
  while (atomic_load(&commands_running))
  {
    fgets(command, sizeof(command), stdin);

    if (strncmp(command, "stop", 4) == 0) {
      close_command();
      break;
    }

    REXWARN("Commando nao encontrado!");
  }

  return NULL;
}

void close_command() {
  REXTRACE("server commands: closure requested");
  atomic_store(&commands_running, false);
  server_close();
}