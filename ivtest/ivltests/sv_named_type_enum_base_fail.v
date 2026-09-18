// Check that an undefined enum base type fails during elaboration.

module test;
  typedef enum missing_type { VALUE } value_type; // Error: missing_type is not a type.
endmodule
