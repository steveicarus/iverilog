// Check that an undefined typedef base type fails during elaboration.

module test;
  typedef missing_type value_type; // Error: missing_type is not a type.
  value_type value;
endmodule
