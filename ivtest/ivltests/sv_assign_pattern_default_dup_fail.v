// Check that only one default item is allowed in a pattern.

module test;

  int a[4];

  initial begin
    a = '{default: 1, default: 2};
  end

endmodule
