// Check that a module instance hides an outer typedef after its declaration.

typedef logic [7:0] i_m;

module M;
endmodule

module test;
  M i_m(); // Instantiates module and shadows the outer typedef.
  i_m value; // Error: i_m names the module instance, not a type.
endmodule
