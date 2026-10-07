// Check that a real variable can be used as a wait condition.

module test;

  real condition;
  integer reached;
  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    wait (condition);
    reached = 1;
    wait (!condition);
    reached = 2;
    wait (condition);
    reached = 3;
  end

  initial begin
    failed = 1'b0;
    reached = 0;
    condition = 0.0;
    #1;
    `check(reached, 0);
    condition = 0.25;
    #1;
    `check(reached, 1);
    condition = 0.0;
    #1;
    `check(reached, 2);
    condition = -0.25;
    #1;
    `check(reached, 3);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
