// Check that static unpacked arrays can not be used in single operand l-value concatenations.

module test;

  int x;
  int u[0:0];

  initial begin
    {u} = x;
  end

endmodule
