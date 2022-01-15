// Regression test for SF bug 947 : Procedural continuous assignment
// affects other structural connections to source vector.

`timescale 1ns/1ps

module test();

wire delay0;
wire delay1;
wire delay2;
reg  select;
reg  out;

assign #100 delay0 = 1;
assign #100 delay1 = delay0;
assign #100 delay2 = delay1;

always @(select) begin
  if (select)
    assign out = delay2;
  else
    assign out = delay0;
end

initial begin
  $monitor($time,, delay0,, delay1,, delay2,, out);
  select = 0;
  #250;
  select = 1;
  #300;
  if ((delay0 == 1) && (delay1 == 1) && (delay2 == 1) && (out == 1))
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
