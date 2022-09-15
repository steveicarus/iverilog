// Check that declaring an integer typed net for a signal that was previously
// declared as a non-ANSI module port is an error. Even if the types for both
// declarations are the same.

module test(x);
  input integer x;
  wire integer x;
endmodule
