// Check that packed dimensions cannot be added to an unpacked named type.

typedef logic byte_array_t [1:0];

module test;
  localparam integer WIDTH = $bits(byte_array_t [3:0]);
endmodule
