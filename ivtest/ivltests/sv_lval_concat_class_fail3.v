// Check that class objects can not be used in continuous assignment l-value concatenations.

module test;

  class C;
  endclass

  int x;
  C c;
  int y;

  assign {x, c} = y;

endmodule
