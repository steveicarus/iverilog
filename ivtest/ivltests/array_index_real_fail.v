// Check that real expressions can not be used as array indices.

module test;

  reg [1:0] a[1:0];
  real r;

  initial begin
    a[r] = 2'b10;
  end

endmodule
