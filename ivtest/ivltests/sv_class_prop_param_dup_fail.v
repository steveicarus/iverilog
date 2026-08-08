// Check that a class property and parameter cannot have the same name.

module test;

  class C;
    int value;
    localparam int value = 1;
  endclass

endmodule
