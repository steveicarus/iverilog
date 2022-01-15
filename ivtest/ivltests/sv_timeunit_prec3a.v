/*
 * Check the basic parsing.
 */

// A global timeunit and timeprecision are OK
timeunit 100us;
timeprecision 1us;

/*
 * Check the various timeunit/precision combinations (this is valid SV syntax).
 */
// A local time unit is OK.
module check_tu;
  timeunit 10us;
endmodule

// A local time precision is OK.
module check_tp;
  timeprecision 10us;
endmodule

// Both a local time unit and precision are OK.
module check_tup;
  timeunit 10us;
  timeprecision 10us;
endmodule

// Both a local time unit and precision are OK (check both orders).
module check_tpu;
  timeprecision 10us;
  timeunit 10us;
endmodule

/*
 * Now do the same with repeat declarations (this is valid SV syntax).
 */
// A global timeunit and timeprecision are OK
timeunit 100us;
timeprecision 1us;

// A local time unit is OK.
module check_tu_d;
  timeunit 10us;
  timeunit 10us;
endmodule

// A local time precision is OK.
module check_tp_d;
  timeprecision 10us;
  timeprecision 10us;
endmodule

// Both a local time unit and precision are OK.
module check_tup_d;
  timeunit 10us;
  timeprecision 10us;
  timeunit 10us;
  timeprecision 10us;
endmodule

// Both a local time unit and precision are OK (check both orders).
module check_tpu_d;
  timeprecision 10us;
  timeunit 10us;
  timeprecision 10us;
  timeunit 10us;
endmodule

/*
 * Now check all the valid timeunit and time precision values.
 */
module check_100s;
  timeunit 100s;
  timeprecision 100s;
endmodule
module check_10s;
  timeunit 10s;
  timeprecision 10s;
endmodule
module check_1s;
  timeunit 1s;
  timeprecision 1s;
endmodule
module check_100ms;
  timeunit 100ms;
  timeprecision 100ms;
endmodule
module check_10ms;
  timeunit 10ms;
  timeprecision 10ms;
endmodule
module check_1ms;
  timeunit 1ms;
  timeprecision 1ms;
endmodule
module check_100us;
  timeunit 100us;
  timeprecision 100us;
endmodule
module check_10us;
  timeunit 10us;
  timeprecision 10us;
endmodule
module check_1us;
  timeunit 1us;
  timeprecision 1us;
endmodule
module check_100ns;
  timeunit 100ns;
  timeprecision 100ns;
endmodule
module check_10ns;
  timeunit 10ns;
  timeprecision 10ns;
endmodule
module check_1ns;
  timeunit 1ns;
  timeprecision 1ns;
endmodule
module check_100ps;
  timeunit 100ps;
  timeprecision 100ps;
endmodule
module check_10ps;
  timeunit 10ps;
  timeprecision 10ps;
endmodule
module check_1ps;
  timeunit 1ps;
  timeprecision 1ps;
endmodule
module check_100fs;
  timeunit 100fs;
  timeprecision 100fs;
endmodule
module check_10fs;
  timeunit 10fs;
  timeprecision 10fs;
endmodule
module check_1fs;
  timeunit 1fs;
  timeprecision 1fs;
endmodule

module check1;

initial begin
  $printtimescale(check_100s);
  $printtimescale(check_10s);
  $printtimescale(check_1s);
  $printtimescale(check_100ms);
  $printtimescale(check_10ms);
  $printtimescale(check_1ms);
  $printtimescale(check_100us);
  $printtimescale(check_10us);
  $printtimescale(check_1us);
  $printtimescale(check_100ns);
  $printtimescale(check_10ns);
  $printtimescale(check_1ns);
  $printtimescale(check_100ps);
  $printtimescale(check_10ps);
  $printtimescale(check_1ps);
  $printtimescale(check_100fs);
  $printtimescale(check_10fs);
  $printtimescale(check_1fs);
  $display("");
  $printtimescale(check_tu);
  $printtimescale(check_tp);
  $printtimescale(check_tup);
  $printtimescale(check_tpu);
  $display("");
  $printtimescale(check_tu_d);
  $printtimescale(check_tp_d);
  $printtimescale(check_tup_d);
  $printtimescale(check_tpu_d);
end

endmodule
