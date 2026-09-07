// Check that duplicate no-port instances retain their own source locations.

module M;
endmodule

module test;
  M i_m [1:0],
    i_m [1:0]; // Error: i_m has already been declared in this scope.
endmodule
