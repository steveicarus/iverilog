// Check that two empty connections to the same named port are rejected.

module M(input value);
endmodule

module test;
  M i_m(.value(), .value()); // Error: The port is already bound.
endmodule
