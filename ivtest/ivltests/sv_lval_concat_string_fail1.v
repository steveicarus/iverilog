// Check that strings can not be used in l-value concatenations.

module test;

  int x;
  string s;

  initial begin
    {x, s} = x;
  end

endmodule
