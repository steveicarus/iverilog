// Check that declaring a real typed non-ANSI module port for a signal that was
// previously declared as a variable is an error. Even if the types for both
// declarations are the same.

module test(x);
  real x;
  output real x;
endmodule
