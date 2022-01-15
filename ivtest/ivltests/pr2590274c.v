// We want to print a warning if we find a delay that comes from the
// default timescale (1s) and then one from a given timescale.
// Basically we want to have either the case of no timescales or
// timescales for all delays.
module wo_time(out, in);
  output out;
  input in;

  buf(out, in);

  specify
    (in => out) = 1;
  endspecify
endmodule

module top;
  reg in;
  wire out_wo, out_w;

  wo_time wo(out_wo, in);
  w_time w(out_w, in);

  initial begin
    in = 1'b1;
    #2 $finish(0);
  end

  always @(out_wo) $display("The time in wo_time is: %e", $abstime);
  always @(out_w) $display("The time in w_time is: %e", $abstime);
endmodule

`timescale 1ns/1ns
module w_time(out, in);
  output out;
  input in;

  buf(out, in);

  specify
    (in => out) = 1;
  endspecify
endmodule
