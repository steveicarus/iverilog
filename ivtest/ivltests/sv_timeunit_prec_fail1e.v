// Check a local timeprecision that is too large.
`resetall
module ltp_large;
  timeunit 1ns;
  timeprecision 10ns;
endmodule

