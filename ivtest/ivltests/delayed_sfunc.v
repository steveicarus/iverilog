// This test is mostly to make sure valgrind cleans up correctly.
`timescale 1ns/1ns
module top;
  wire real rtm;
  wire [31:0] res1, res2;
  integer a = 10;

  assign #1 rtm = $realtime;
  assign #1 res1 = $clog2(a);
  lwr dut(res2, a);

  initial begin
    $monitor($realtime,, rtm, res1,, res2,, a);
    #5 a = 20;
  end

endmodule

module lwr(out, in);
  output [31:0] out;
  input [31:0] in;
  wire [31:0] out, in;

  assign out = $clog2(in);

  specify
    (in => out) = (1, 1);
  endspecify
endmodule
