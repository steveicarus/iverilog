// Check that a statement cannot have more than one label.

module test;

  reg value;

  initial A: B: value = 1'b0; // Error: A statement cannot have multiple labels.

endmodule
