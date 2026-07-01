// Check that static unpacked arrays can not be used in l-value concatenations.

module test;

  int x;
  int u[0:0];

  initial begin
    {x, u} = x;
  end

endmodule
