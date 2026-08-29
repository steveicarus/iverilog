// Check that an indexed part select is not a named type dimension.

typedef logic [7:0] byte_t;

module test;
  localparam integer WIDTH = $bits(byte_t [0 +: 2]);
endmodule
