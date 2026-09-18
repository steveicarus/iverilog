// Check that a mixed instance list cannot omit scalar port parentheses.

module M;
endmodule

module test;
  M first [1:0], middle, last();
endmodule
