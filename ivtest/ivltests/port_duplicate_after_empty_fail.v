// Check that an empty named port connection still counts as a binding.

module M(input value);
endmodule

module test;
  M i_m(.value(), .value(1'b1)); // Error: The port is already bound.
endmodule
