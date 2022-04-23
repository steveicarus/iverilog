// Check that declaring an integer typed non-ANSI module port for signal that
// was previously declared as a net is an error. Even if the types for both
// declarations are the same.

module test(x);
  wire integer x;
  input integer x;
endmodule
