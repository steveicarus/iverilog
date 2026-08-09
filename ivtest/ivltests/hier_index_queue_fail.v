// Check that a queue bound is not allowed in a hierarchy identifier.

module test;
  reg [3:0] value;
  initial $display("%b", value[$:1]);
endmodule
