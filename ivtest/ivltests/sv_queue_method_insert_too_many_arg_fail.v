// Check that queue insert() rejects too many arguments.

module test;
  int q[$];

  initial begin
    q.insert(0, 1, 2);
  end
endmodule
