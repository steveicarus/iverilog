// Check that a type in a disable target path is rejected.

module test;

  typedef logic value;

  initial disable test.value;

endmodule
