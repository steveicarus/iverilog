// Check that queues can not be used in continuous assignment l-value concatenations.

module test;

  int x;
  int q[$];
  int y;

  assign {x, q} = y;

endmodule
