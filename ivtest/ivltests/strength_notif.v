// Check that NOTIF outputs preserve ambiguous strength changes.

module test;

  reg data, enable;
  wire mid0, mid1, result0, result1;
  reg failed;

  notif0 (mid0, data, enable);
  notif1 (mid1, data, enable);
  nmos (result0, mid0, 1'b1);
  nmos (result1, mid1, 1'b1);
  assign (weak0, weak1) result0 = ~data;
  assign (weak0, weak1) result1 = ~data;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    data = 1'b1;
    enable = 1'bx;
    #1;
    `check(result0, 1'b0);
    `check(result1, 1'b0);

    // Both intermediate logic values stay X when the data changes.
    data = 1'b0;
    #1;
    `check(result0, 1'b1);
    `check(result1, 1'b1);

    enable = 1'bz;
    #1;
    `check(result0, 1'b1);
    `check(result1, 1'b1);

    data = 1'b1;
    #1;
    `check(result0, 1'b0);
    `check(result1, 1'b0);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
