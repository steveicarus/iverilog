// Check that an array index without a key and no default is an error.

module test;

  int a[4];

  initial begin
    a = '{1: 1, 2: 2};
  end

endmodule
