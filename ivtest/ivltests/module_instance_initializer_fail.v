// Check that a module instance cannot have a variable initializer.

module M;
endmodule

module test;
  M i_m1 [1:0],
    i_m2 [1:0] = 1, // Error: Module instances cannot have variable initializers.
    i_m3();
endmodule
