// Check that declaring both a net and a variable for a signal that was
// previously declared as a non-ANSI module port is an error.

module test(x);
  input x;
  wire x;
  reg x;
endmodule
