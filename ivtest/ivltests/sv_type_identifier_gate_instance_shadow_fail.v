// Check that a gate instance hides an outer typedef after its declaration.

typedef logic [7:0] G;

module test;
  wire out;

  and G(out, 1'b1, 1'b1); // Instantiates gate and shadows the outer typedef.
  G value; // Error: G names the gate instance, not a type.
endmodule
