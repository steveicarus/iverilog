// Check that an undefined named variable type fails during elaboration.

module test;
  var missing_type value; // Error: missing_type is not a type.
endmodule
