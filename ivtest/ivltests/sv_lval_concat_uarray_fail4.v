// Check that static unpacked arrays can not be used in single operand continuous assignment l-value concatenations.

module test;

  int u[0:0];
  int y;

  assign {u} = y;

endmodule
