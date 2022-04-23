// Check that declaring multiple non-ANSI module output ports with an explicit
// type is an error. Even if the types are the same.

module test(x);
  output integer x;
  output integer x;

endmodule
