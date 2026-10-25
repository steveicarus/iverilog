// Check that an empty wildcard override counts as an explicit binding.

module M(input value);
endmodule

module test;
  wire value = 1'b0;
  M i_m(.*, .value(), .value(1'b1)); // Error: The port is already bound.
endmodule
