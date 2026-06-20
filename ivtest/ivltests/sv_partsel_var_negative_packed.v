// Check variable selects of packed arrays with negative bounds.

module test;

  reg failed;
  reg [-8:-1][3:0] a;
  reg signed [2:0] i;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    a = '0;

    i = -1;
    a[i] = 4'ha;
    i = -2;
    a[i] = 4'h5;

    `check(a[-1], 4'ha);
    `check(a[-2], 4'h5);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
