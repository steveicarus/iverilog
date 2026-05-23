// Check that net arrays can be initialized during declaration.

module test;

  reg failed;

  wire [3:0] a[0:1] = '{4'h1, 4'h2};

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;

    `check(a[0], 4'h1)
    `check(a[1], 4'h2)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
