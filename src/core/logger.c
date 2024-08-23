#include "logger.h"

#include "logger.h"
#include "asserts.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

b8 logger_initialize() {
  REXINFO("Logger system initialized!");
  return true;
}

void logger_shutdown() {
  
}

void log_output(log_level level, const char* message, ...) {
  const char* level_strings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};

  const i32 msg_length = 1024;

  char out_message[1024];
  memset(out_message, 0, sizeof(out_message));

  va_list arg_ptr;
  va_start(arg_ptr, message);
  vsnprintf(out_message, msg_length, message, arg_ptr);
  va_end(arg_ptr);

  char out_message2[1024];
  sprintf(out_message2, "%s%s\n", level_strings[level], out_message);

  const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
  printf("\033[%sm%s\033[0m", colour_strings[level], out_message2);
}

void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line) {
  log_output(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: '%s', in file: %s, line: %d\n", expression, message, file, line);
}