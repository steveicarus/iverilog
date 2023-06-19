// Check that procedural assignment of unpacked array assignment patterns is
// supported and a entries are assigned in the right order.

module test;

  bit failed;

  `define check(val, exp) do \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, `"val`", exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  int x[3:0];
  int y[0:3];
  int z[4];

  initial begin
    x = '{1'b1, 1 + 1, 3.3, "TEST"};
    y = '{1'b1, 1 + 1, 3.3, "TEST"};
    z = '{1'b1, 1 + 1, 3.3, "TEST"};

    `check(x[0], 1413829460);
    `check(x[1], 3);
    `check(x[2], 2);
    `check(x[3], 1);

    `check(y[0], 1);
    `check(y[1], 2);
    `check(y[2], 3);
    `check(y[3], 1413829460);

    `check(z[0], 1);
    `check(z[1], 2);
    `check(z[2], 3);
    `check(z[3], 1413829460);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
