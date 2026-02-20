// Test for GitHub issue #1170
// tgt-sizer should work with SystemVerilog 2012 ($unit scope)
module test;
  logic [7:0] data;
  assign data = 8'hAA;
endmodule
