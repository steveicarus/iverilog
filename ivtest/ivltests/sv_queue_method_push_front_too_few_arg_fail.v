// Check that queue push_front() rejects too few arguments.

module test;
  int q[$];

  initial begin
    q.push_front();
  end
endmodule
