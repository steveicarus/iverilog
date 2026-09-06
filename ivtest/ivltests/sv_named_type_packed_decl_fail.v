// Check that an undefined packed named type fails during elaboration.

module test;
  missing_type [3:0] value; // Error: missing_type is not a type.
endmodule
