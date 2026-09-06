// Check that an undefined type on an initialized continued port fails elaboration.

module test(
  input logic first,
  missing_type value = 1'b0 // Error: missing_type is not a type.
);
endmodule
