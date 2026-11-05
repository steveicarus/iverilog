// Check that a key outside of the array range is an error.

module test;

  int a[4];

  initial begin
    a = '{4: 1, default: 0};
  end

endmodule
