// Check that strings can not be used in continuous assignment l-value concatenations.

module test;

  int x;
  string s;
  int y;

  assign {x, s} = y;

endmodule
