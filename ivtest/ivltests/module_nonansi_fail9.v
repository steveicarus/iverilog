// Check that declaring an integer typed variable for a signal that was previously
// declared as a real typed non-ANSI module port is an error.

module test(x);
  output real x;
  integer x;
endmodule
