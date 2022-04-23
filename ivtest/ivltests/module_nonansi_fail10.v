// Check that declaring a non-ANSI module port with an explicit type for a
// signal that was previously declared as a real variable is an error.

module test(x);
  real x;
  output integer x;
endmodule
