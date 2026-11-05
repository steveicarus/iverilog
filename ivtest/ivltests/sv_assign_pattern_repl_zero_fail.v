// Check that a replication count of zero is an error.

module test;

  int a[4];

  initial begin
    a = '{0{1}};
  end

endmodule
