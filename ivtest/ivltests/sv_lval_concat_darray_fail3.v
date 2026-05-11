// Check that dynamic arrays can not be used in continuous assignment l-value concatenations.

module test;

  int x;
  int d[];
  int y;

  assign {x, d} = y;

endmodule
