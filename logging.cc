#include "logging.h"

#include <algorithm>
#include <cstdio>

// By default only errors are logged
static uint8_t severity_setting = 3;

void log(LogSeverity severity, const char * message, ...) {
      const uint8_t numeric_message_severity = static_cast<uint8_t>(severity);

      // Don't log message with low severity if high severity was configured
      if(numeric_message_severity < severity_setting) {
            return;
      }

      // errors should be logged to stderr instead of stdout
      FILE * outfd = numeric_message_severity < 3 ? stdout : stderr;

      // Write message to stdout
      va_list args;
      va_start(args, message);
      vfprintf(outfd, message, args);
      va_end(args);
}

void set_min_log_severity(LogSeverity severity) {
      set_min_log_severity(static_cast<uint8_t>(severity));
}

void set_min_log_severity(uint8_t severity) {
      severity_setting = severity;
}
