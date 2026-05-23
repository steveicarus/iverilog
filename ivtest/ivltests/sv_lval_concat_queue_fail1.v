// Check that queues can not be used in l-value concatenations.

module test;

  int x;
  int q[$];

  initial begin
    {x, q} = x;
  end

endmodule
