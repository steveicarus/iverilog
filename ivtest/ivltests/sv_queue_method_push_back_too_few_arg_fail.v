// Check that queue push_back() rejects too few arguments.

module test;
  int q[$];

  initial begin
    q.push_back();
  end
endmodule
