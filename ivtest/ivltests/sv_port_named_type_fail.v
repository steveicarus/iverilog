// Check that an undefined named port type fails during elaboration.

module test(
  input missing_type value // Error: missing_type is not a type.
);

endmodule
