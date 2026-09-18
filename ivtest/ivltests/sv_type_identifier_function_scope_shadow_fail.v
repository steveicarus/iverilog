// Check that a function scope hides an outer typedef.

typedef reg [7:0] T;

module test;

  T x;

  function T;
    T = 0;
  endfunction

endmodule
