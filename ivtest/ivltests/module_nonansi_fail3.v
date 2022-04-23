// Check that declaring a variable multiple times for a signal that was
// previously declared as a non-ANSI module port is an error.

module test(x);
  output x;
  reg x;
  reg x;
endmodule
