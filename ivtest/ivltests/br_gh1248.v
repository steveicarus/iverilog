`timescale 1ns/1ps

module tb;
  reg in;
  wire out;

  top dut(out, in);

  initial begin
    $monitor("%.3f: %b %b %b",$realtime,out,dut.intv,in);
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
  wire [2:0] intv;

  assign intv[0] = in;
  buff i1 (.Y(intv[1]), .A(intv[0]));
  buff i2 (.Y(intv[2]), .A(intv[1]));
  assign out = intv[2];
endmodule

`celldefine
module buff(output wire Y, input wire A);
  buf (Y, A);

  specify
    (A => Y) = 0;
  endspecify

endmodule
`endcelldefine
