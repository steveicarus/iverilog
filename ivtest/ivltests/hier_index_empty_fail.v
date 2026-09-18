// Check that an empty index is not allowed in a hierarchy identifier.

module test;
  reg [3:0] value;
  initial $display("%b", value[]);
endmodule
