// Check that a class property and type cannot have the same name.

module test;

  class C;
    int value;
    typedef int value;
  endclass

endmodule
