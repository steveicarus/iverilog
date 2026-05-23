// Check that dynamic arrays can not be used in l-value concatenations.

module test;

  int x;
  int d[];

  initial begin
    {x, d} = x;
  end

endmodule
