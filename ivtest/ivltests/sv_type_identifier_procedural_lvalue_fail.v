// Check that a type cannot be used as a procedural l-value.

module container;
  typedef logic value;
endmodule

module test;

  container i_container();

  initial i_container.value = 1;

endmodule
