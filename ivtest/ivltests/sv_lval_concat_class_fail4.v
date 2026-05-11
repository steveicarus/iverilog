// Check that class objects can not be used in single operand continuous assignment l-value concatenations.

module test;

  class C;
  endclass

  C c;
  int y;

  assign {c} = y;

endmodule
