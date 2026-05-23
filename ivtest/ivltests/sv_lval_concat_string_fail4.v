// Check that strings can not be used in single operand continuous assignment l-value concatenations.

module test;

  string s;
  int y;

  assign {s} = y;

endmodule
