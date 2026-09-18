// Check that a type cannot be referenced through a hierarchical path.

module test #(parameter type T = logic);

  localparam integer WIDTH = $bits(test.T);

endmodule
