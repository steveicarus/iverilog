// We want to print a warning if we find a delay that comes from the
// default timescale (1s) and then one from a given timescale.
// Basically we want to have either the case of no timescales or
// timescales for all delays.
module wo_time;
  reg in;
  wire #1 out = in;

  initial begin
    in = 1'b1;
    #2 $finish(0);
  end

  always @(out) $display("The time in %m is: %e", $abstime);
endmodule

`timescale 1ns/1ns
module w_time;
  reg in;
  wire #1 out = in;

  initial in = 1'b1;
  always @(out) $display("The time in %m is: %e", $abstime);
endmodule
