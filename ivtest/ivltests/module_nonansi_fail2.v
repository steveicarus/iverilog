// Check that declaring a net multiple times for a signal that was previously
// declared as a non-ANSI module port is an error.

module test(x);
  input x;
  wire x;
  wire x;
endmodule
