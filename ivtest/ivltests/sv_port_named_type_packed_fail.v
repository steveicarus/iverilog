// Check that an undefined packed type on a continued port fails elaboration.

module test(
  input logic first,
  missing_type [3:0] value // Error: missing_type is not a type.
);
endmodule
