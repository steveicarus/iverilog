// A global timeunit and local time precision.

timeunit 10s;

module gtu_ltp1;
  timeprecision 10ps;
endmodule

module gtu_ltp2;
  timeprecision 1ns;
endmodule

`timescale 1s/1s
module check4;

initial begin
  $printtimescale(gtu_ltp1);
  $printtimescale(gtu_ltp2);
end

endmodule
