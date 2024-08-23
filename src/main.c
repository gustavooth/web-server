#include "defines.h"
#include "web/server.h"
#include "web/commands.h"
#include "core/logger.h"
#include "core/events.h"

int main() {
  logger_initialize();
  event_initialize();
  server_run();
  server_commands_run();

  while (true) {}
  return 0;
}
