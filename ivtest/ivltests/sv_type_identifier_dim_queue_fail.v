// Check that a queue suffix is not a named type dimension here.

typedef logic [7:0] byte_t;

module test;
  localparam integer WIDTH = $bits(byte_t [$:3]);
endmodule
