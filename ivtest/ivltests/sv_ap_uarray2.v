// Check that procedural assignment of unpacked array assignment patterns to
// multi-dimensional arrays is supported and entries are assigned in the right
// order.

module test;

  bit passed;

  `define check(val, exp) do \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, `"val`", exp, val); \
      passed = 1'b0; \
    end \
  while(0)

  int x[1:0][1:0];
  int y[1:0][0:1];
  int z[2][2];

  initial begin
    passed = 1'b1;
    x = '{'{1'b1, 1 + 1}, '{3.3, "TEST"}};
    y = '{'{1'b1, 1 + 1}, '{3.3, "TEST"}};
    z = '{'{1'b1, 1 + 1}, '{3.3, "TEST"}};

    `check(x[0][0], 1413829460);
    `check(x[0][1], 3);
    `check(x[1][0], 2);
    `check(x[1][1], 1);

    `check(y[0][0], 3);
    `check(y[0][1], 1413829460);
    `check(y[1][0], 1);
    `check(y[1][1], 2);

    `check(z[0][0], 1);
    `check(z[0][1], 2);
    `check(z[1][0], 3);
    `check(z[1][1], 1413829460);

    if (passed) begin
      $display("PASSED");
    end
  end

endmodule
