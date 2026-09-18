// Check that a descending indexed part select is not a dimension.

module test;
  reg value[1 -: 2];
endmodule
