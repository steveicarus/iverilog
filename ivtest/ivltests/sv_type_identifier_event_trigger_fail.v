// Check that a type cannot be used as a named event.

module container;
  typedef logic value;
endmodule

module test;

  container i_container();

  initial -> i_container.value;

endmodule
