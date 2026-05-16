// Check that dynamic arrays can not be used in single operand l-value concatenations.

module test;

  int x;
  int d[];

  initial begin
    {d} = x;
  end

endmodule
