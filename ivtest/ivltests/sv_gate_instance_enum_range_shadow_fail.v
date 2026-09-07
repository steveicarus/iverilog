// Check that a gate instance hides an outer parameter during enum elaboration.

parameter i_g = 8;

module test;
  wire out;
  and i_g(out, 1'b1, 1'b1);

  // Enum ranges are resolved during scope elaboration, before signals.
  typedef enum logic [i_g-1:0] { A = 0 } T; // Error: i_g is a gate instance, not a constant.
endmodule
