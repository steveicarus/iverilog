// Check that declaring a real typed variable for a signal that was previously
// declared as a non-ANSI module port is an error. Even if the types for both
// declarations are the same.

module test(x);
  output real x;
  real x;
endmodule
