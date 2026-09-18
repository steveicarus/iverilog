// Check that duplicate class property names are rejected.

module test;

  class C;
    int value;
    static int value;
  endclass

endmodule
