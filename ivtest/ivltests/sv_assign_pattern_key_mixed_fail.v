// Check that keyed and positional items can not be mixed.

module test;

  int a[4];

  initial begin
    a = '{2: 2, 1, 3};
  end

endmodule
