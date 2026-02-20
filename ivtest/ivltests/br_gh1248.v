`timescale 1ns/1ps

module tb;
  reg in;
  wire out;

  top dut(out, in);

  initial begin
    $monitor("%.3f: %b %b %b",$realtime,out,dut.int,in);
    $sdf_annotate("ivltests/br_gh1248.sdf");
    #1;
    in = 1'b0;
    #1;
    in = 1'b1;
    #1;
    in = 1'b0;
    #1;
  end
endmodule

module top(output wire out, input wire in);
  wire [2:0] int;

  assign int[0] = in;
  buff i1 (.Y(int[1]), .A(int[0]));
  buff i2 (.Y(int[2]), .A(int[1]));
  assign out = int[2];
endmodule

`celldefine
module buff(output wire Y, input wire A);
  buf (Y, A);

  specify
    (A => Y) = 0;
  endspecify
 
endmodule
`endcelldefine
