/*
 * Check declarations and repeat declarations in nested modules.
 */
timeunit 100us;
timeprecision 1us;

// A local time unit is OK.
module check_tu_nest;
  timeunit 10us;
  module nested;
    timeunit 100us;
    timeunit 100us;
  endmodule
  timeunit 10us;
endmodule

// A local time precision is OK.
module check_tp_nest;
  timeprecision 10us;
  module nested;
    timeprecision 1us;
    timeprecision 1us;
  endmodule
  timeprecision 10us;
endmodule

// Both a local time unit and precision are OK.
module check_tup_nest;
  timeunit 10us;
  timeprecision 10us;
  module nested;
    timeunit 100us;
    timeprecision 1us;
    timeunit 100us;
    timeprecision 1us;
  endmodule
  timeunit 10us;
  timeprecision 10us;
endmodule

// Both a local time unit and precision are OK (check both orders).
module check_tpu_nest;
  timeprecision 10us;
  timeunit 10us;
  module nested;
    timeprecision 1us;
    timeunit 100us;
    timeprecision 1us;
    timeunit 100us;
  endmodule
  timeprecision 10us;
  timeunit 10us;
endmodule

module check2;

initial begin
  $printtimescale(check_tu_nest);
  $printtimescale(check_tp_nest);
  $printtimescale(check_tup_nest);
  $printtimescale(check_tpu_nest);
  $display("");
  $printtimescale(check_tu_nest.nested);
  $printtimescale(check_tp_nest.nested);
  $printtimescale(check_tup_nest.nested);
  $printtimescale(check_tpu_nest.nested);
end

endmodule
