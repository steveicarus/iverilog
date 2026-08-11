// Check that testing an invalid type parameter does not elaborate its prefix.

module test;
  wire [$bits(T)-1:0] value;
  parameter type T = value.invalid;
endmodule
