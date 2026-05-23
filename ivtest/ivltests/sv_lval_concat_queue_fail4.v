// Check that queues can not be used in single operand continuous assignment l-value concatenations.

module test;

  int q[$];
  int y;

  assign {q} = y;

endmodule
