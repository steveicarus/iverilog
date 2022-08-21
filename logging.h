#pragma once

#include <stdarg.h>
#include <cstdint>

// Log Severity is organized in 4 levels:
//    0 / Debug   : log info + log internal information that is exclusively useful to developers with access to and knowledge of the code
//    1 / Info    : log warnings + log internal information that might be interesting to some users in special situations. This is equivalent to what used to be logged with verbose_flag.
//    2 / Warning : log errors + logs warnings for cases where there might be an error
//    3 / Error   : only log errors that result in termination of the verilog simulation
enum class LogSeverity {
      Debug,
      Info,
      Warning,
      Error
};

void log(LogSeverity severity, const char * message, ...);
void set_min_log_severity(LogSeverity severity);
void set_min_log_severity(uint8_t severity);
