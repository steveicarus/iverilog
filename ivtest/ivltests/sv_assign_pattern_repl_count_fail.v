// Check that a replicated pattern with the wrong element count is an error.

module test;

  int a[4];

  initial begin
    a = '{3{1}};
  end

endmodule
