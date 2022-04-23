// Check that declaring multiple non-ANSI module ports with the same name is an
// error. Even if they both have an implicit type.

module test(x);
  input x;
  input x;
endmodule
