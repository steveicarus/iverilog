/*
 * Check declarations and repeat declarations in nested modules.
 */
timeunit 100us/1us;

// Both a local time unit and precision are OK.
module check_tup_nest;
  timeunit 10us / 10us;
  module nested;
    timeunit 100us / 1us;
    timeunit 100us / 1us;
  endmodule
  timeunit 10us / 10us;
endmodule

module check2();

initial begin
  $printtimescale(check_tup_nest);
  $printtimescale(check_tup_nest.nested);
end

endmodule
