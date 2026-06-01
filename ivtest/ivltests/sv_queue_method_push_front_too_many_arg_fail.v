// Check that queue push_front() rejects too many arguments.

module test;
  int q[$];

  initial begin
    q.push_front(1, 2);
  end
endmodule
