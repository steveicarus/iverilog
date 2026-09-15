// Check that releasing a real net propagates the value driven while forced.

module test;

  real source;
  wire real value = source;
  wire real observed = value + 1.0;
  reg failed;

  `define check(val, exp) \
    if (val != exp) begin \
      $display("FAILED(%0d). '%s' expected %f, got %f", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    source = 2.0;
    #1;
    `check(observed, 3.0);

    force value = 10.0;
    #1;
    `check(observed, 11.0);

    source = 3.0;
    #1;
    `check(observed, 11.0);

    release value;
    #1;
    `check(observed, 4.0);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
