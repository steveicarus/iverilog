// Check that a negative replication count is an error.

module test;

  int a[4];

  initial begin
    a = '{-4{1}};
  end

endmodule
