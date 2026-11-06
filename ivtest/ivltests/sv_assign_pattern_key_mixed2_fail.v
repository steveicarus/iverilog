// Check that keyed and positional items can not be mixed.

module test;

  int a[4];

  initial begin
    a = '{1, 2, default: 0};
  end

endmodule
