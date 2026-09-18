// Check that an undefined named parameter type without the `parameter`
// keyword fails during elaboration.

module test #(
  missing_type value = 0
);

endmodule
