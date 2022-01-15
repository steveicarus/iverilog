/*
 * Check that errors are caught.
 */

timeunit 100us/1us;

// Repeated declarations must match the initial declarations.
timeunit 1ms/1ns;

// A local time unit/precision is OK, but a repeat must match.
module check_tup_d_e;
  timeunit 10us/10us;
  timeunit 1us/1us;
  timeunit 1us;
  timeprecision 1us;
endmodule

// A repeat time unit/precision is only allowed if an initial one is given.
module check_tup_m_e;
  integer foo;
  timeunit 10us/10us;
endmodule

// A local time unit is OK and a repeat is OK, but this is not a prec decl.
module check_tu_d_e;
  timeunit 10us;
  timeunit 10us/1us;
endmodule

// A local time prec is OK and a repeat is OK, but this is not a unit decl.
module check_tp_d_e;
  timeprecision 1us;
  timeunit 10us/1us;
endmodule

/* Check some invalid values */

// Only a power of 10 is allowed.
timeunit 200s/200s;
// Too many zeros (only allow 0 - 2).
timeunit 1000s/1000s;
// This actually trips as an invalid scale of '2s'.
timeunit 12s/12s;
// This needs to be checked. The base time_literal supports this, but
// for now timeunit/precision code does not.
timeunit 1_0s/1_0s;
