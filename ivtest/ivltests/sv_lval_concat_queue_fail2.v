// Check that queues can not be used in single operand l-value concatenations.

module test;

  int x;
  int q[$];

  initial begin
    {q} = x;
  end

endmodule
