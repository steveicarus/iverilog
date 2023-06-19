// Check that procedural assignment of unpacked real array assignment patterns
// to multi-dimensional arrays is supported and entries are assigned in the
// right order.

module test;

  bit failed;

  `define check(val, exp) do \
    if (val != exp) begin \
      $display("FAILED(%0d). '%s' expected %0f, got %0f", `__LINE__, `"val`", exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  real x[1:0][1:0];
  real y[1:0][0:1];
  real z[2][2];

  initial begin
    x = '{'{1'b1, 1 + 1}, '{3.3, "TEST"}};
    y = '{'{1'b1, 1 + 1}, '{3.3, "TEST"}};
    z = '{'{1'b1, 1 + 1}, '{3.3, "TEST"}};

    `check(x[0][0], 1413829460.0);
    `check(x[0][1], 3.3);
    `check(x[1][0], 2.0);
    `check(x[1][1], 1.0);

    `check(y[0][1], 1413829460.0);
    `check(y[0][0], 3.3);
    `check(y[1][1], 2.0);
    `check(y[1][0], 1.0);

    `check(z[0][0], 1.0);
    `check(z[0][1], 2.0);
    `check(z[1][0], 3.3);
    `check(z[1][1], 1413829460.0);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
