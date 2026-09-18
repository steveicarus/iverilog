// Check that duplicate names in a class property declaration are rejected.

module test;

  class C;
    int value, value;
  endclass

endmodule
