// We want to print a warning if we find a delay that comes from the
// default timescale (1s) and then one from a given timescale.
// Basically we want to have either the case of no timescales or
// timescales for all delays.
module wo_time;
  initial #1 $display("The time in %m is: %e", $abstime);
endmodule

`timescale 1ns/1ns
module w_time;
  initial #1 $display("The time in %m is: %e", $abstime);
endmodule
