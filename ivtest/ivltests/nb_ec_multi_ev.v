// Check that non-blocking event control assignments with multiple events in the
// event control expression are supported.

module test;
  reg failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED. Expected %d, got %d.", exp, val); \
      failed = 1'b1; \
    end

  integer x = 0;

  event e1, e2, e3;

  initial begin
    // Any of them should trigger the event
    x <= @(e1 or e2 or e3) x + 1;
    #1
    `check(x, 0);
    ->e1;
    `check(x, 1);

    x <= @(e1 or e2 or e3) x + 1;
    #1
    `check(x, 1);
    ->e2;
    `check(x, 2);

    // Alternative syntax, but still the same behavior
    x <= @(e1, e2, e3) x + 1;
    #1
    `check(x, 2);
    ->e3;
    `check(x, 3);

    // In combination with repeat
    x <= repeat(3) @(e1, e2, e3) x + 1;
    #1
    `check(x, 3);
    ->e1;
    `check(x, 3);
    ->e2;
    `check(x, 3);
    ->e3;
    `check(x, 4);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
