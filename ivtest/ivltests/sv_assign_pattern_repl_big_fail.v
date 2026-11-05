// Check that an oversized replication count is rejected.

module test;

  int a[4];

  initial begin
    a = '{100000000{1}};
  end

endmodule
