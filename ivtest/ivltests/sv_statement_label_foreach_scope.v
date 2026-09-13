// Check that disabling inner and outer labels continues and exits a foreach loop.

module test;

  reg failed;
  reg [7:0] values [0:2];
  integer count_before;
  integer count_after;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    count_before = 0;
    count_after = 0;

    OUTER: (* keep = 1 *) foreach (values[i]) begin : INNER
      count_before = count_before + 1;
      if (i == 0) disable INNER;
      disable OUTER;
      count_after = count_after + 1;
    end

    `check(count_before, 2);
    `check(count_after, 0);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
