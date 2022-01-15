`timescale 1 ns / 1 ns

module test(
  inout wire [1:0] B0,
  input wire       WR,
  input wire [1:0] IN
);

assign B0 = WR ? 2'bz : IN;

initial $monitor("%b %b %b %0t", IN, WR, B0, $time);

endmodule
