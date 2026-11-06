// Check that the replication count of a pattern must be constant.

module test;

  int a[4];
  int n = 4;

  initial begin
    a = '{n{1}};
  end

endmodule
