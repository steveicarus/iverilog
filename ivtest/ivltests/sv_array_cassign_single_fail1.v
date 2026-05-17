// Check that scalar expressions can not be continuously assigned to single
// element unpacked arrays.

module test;

  wire [31:0] a[0:0];

  assign a = 1;

endmodule
