// Check that continuous assignment of unpacked array assignment patterns to
// single element arrays is supported.

module test;

  reg failed;
  integer a[0:0];

  assign a = '{42};

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    #1;
    `check(a[0], 42);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
