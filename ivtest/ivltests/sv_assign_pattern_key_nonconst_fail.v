// Check that the key of an array pattern item must be constant.

module test;

  int a[4];
  int i;

  initial begin
    a = '{i: 1, default: 0};
  end

endmodule
