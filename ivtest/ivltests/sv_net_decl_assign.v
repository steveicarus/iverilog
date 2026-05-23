// Check that net declarations can mix initialized and uninitialized entries.

module test;

  reg failed;

  wire [3:0] a, b = 4'h5;
  wire [3:0] c = 4'ha, d;

  assign a = b;
  assign d = c;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;

    `check(a, 4'h5)
    `check(b, 4'h5)
    `check(c, 4'ha)
    `check(d, 4'ha)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
