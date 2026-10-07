// Check real-valued conditions in procedural loops.

module test;

  real condition;
  integer count;
  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    condition = 0.25;
    count = 0;
    while (condition) begin
      count = count + 1;
      condition = 0.0;
    end
    `check(count, 1);

    condition = -0.25;
    for (count = 0; condition; count = count + 1) begin
      condition = 0.0;
    end
    `check(count, 1);

    condition = 2.0;
    count = 0;
    do begin
      count = count + 1;
      if (count == 2) condition = 0.0;
    end while (condition);
    `check(count, 2);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
