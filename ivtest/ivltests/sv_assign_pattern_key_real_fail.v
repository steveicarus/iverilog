// Check that a real value is not accepted as the key of an array pattern item.

module test;

  int a[4];

  initial begin
    a = '{1.5: 1, default: 0};
  end

endmodule
