// Check that specifying the same array index twice is an error.

module test;

  int a[4];

  initial begin
    a = '{1: 1, 1: 2, default: 0};
  end

endmodule
