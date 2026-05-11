// Check that dynamic arrays can not be used in single operand continuous assignment l-value concatenations.

module test;

  int d[];
  int y;

  assign {d} = y;

endmodule
