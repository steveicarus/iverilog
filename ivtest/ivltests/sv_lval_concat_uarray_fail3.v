// Check that static unpacked arrays can not be used in continuous assignment l-value concatenations.

module test;

  int x;
  int u[0:0];
  int y;

  assign {x, u} = y;

endmodule
