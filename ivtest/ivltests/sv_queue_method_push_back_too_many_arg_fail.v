// Check that queue push_back() rejects too many arguments.

module test;
  int q[$];

  initial begin
    q.push_back(1, 2);
  end
endmodule
