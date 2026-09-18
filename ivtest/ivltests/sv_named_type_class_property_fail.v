// Check that an undefined class property type fails during elaboration.

class C;
  missing_type value; // Error: missing_type is not a type.
endclass

module test;
  C value;
endmodule
