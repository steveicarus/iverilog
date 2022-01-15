// Check a global timeprecision that is too large.
`resetall
timeunit 1ns;
timeprecision 10ns;
module gtp_large;
endmodule
