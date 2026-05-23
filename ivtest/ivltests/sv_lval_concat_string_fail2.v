// Check that strings can not be used in single operand l-value concatenations.

module test;

  int x;
  string s;

  initial begin
    {s} = x;
  end

endmodule
