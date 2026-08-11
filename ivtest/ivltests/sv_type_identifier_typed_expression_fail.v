// Check that a type cannot be used as an expression with a required type.

module container;
  typedef logic value;
endmodule

module test;

  container i_container();

  integer values[$];

  initial values = i_container.value;

endmodule
