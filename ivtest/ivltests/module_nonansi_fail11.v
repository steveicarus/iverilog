// Check that declaring multiple non-ANSI module ports with an implicit type and
// the same name is an error. Even if the signal was previously declared as a
// net.

module test(x);
  wire x;
  input x;
  input x;
endmodule
