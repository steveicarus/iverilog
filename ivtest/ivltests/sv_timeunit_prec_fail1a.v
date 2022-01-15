/*
 * Check that errors are caught.
 */

timeunit 100us;
timeprecision 1us;

// Repeated declarations must match the initial declarations.
timeunit 1ms;
timeprecision 1ns;

// A local time unit is OK, but a repeat must match.
module check_tu_d_e;
  timeunit 10us;
  timeunit 1us;
endmodule

// A local time precision is OK, but a repeat must match.
module check_tp_d_e;
  timeprecision 10us;
  timeprecision 1us;
endmodule

// A repeat time unit is only allowed if an initial one is given.
module check_tu_m_e;
  integer foo;
  timeunit 10us;
endmodule

// A repeat time precision is only allowed if an initial one is given.
module check_tp_m_e;
  integer foo;
  timeprecision 10us;
endmodule

// A local time unit is OK and a repeat is OK, but this is not a prec decl.
module check_tup_d_e;
  timeunit 10us;
  timeunit 10us;
  timeprecision 1us;
endmodule

// A local time prec is OK and a repeat is OK, but this is not a unit decl.
module check_tpu_d_e;
  timeprecision 1us;
  timeprecision 1us;
  timeunit 10us;
endmodule

/* Check some invalid values */

// Only a power of 10 is allowed.
timeunit 200s;
timeprecision 200s;
// Too many zeros (only allow 0 - 2).
timeunit 1000s;
timeprecision 1000s;
// This actually trips as an invalid scale of '2s'.
timeunit 12s;
timeprecision 12s;
// This needs to be checked. The base time_literal supports this, but
// for now timeunit/precision code does not.
timeunit 1_0s;
timeprecision 1_0s;
