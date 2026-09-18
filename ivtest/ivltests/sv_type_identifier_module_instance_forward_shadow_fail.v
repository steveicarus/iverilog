// Check that a module instance hides an outer typedef before its declaration.

typedef logic [7:0] i_m;

module M;
endmodule

module test;
  i_m value; // Error: i_m names the module instance throughout this scope.
  M i_m(); // Instantiates module and shadows the outer typedef.
endmodule
