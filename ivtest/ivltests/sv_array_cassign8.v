// Check that continuous assignments to 2-state arrays are supported.

module test;

  bit failed = 1'b0;

  `define check(expr, val) do \
    if (expr !== val) begin \
      $display("FAILED(%0d): `%s`, expected %0d, got %0d", `__LINE__, `"expr`", val, expr); \
      failed = 1'b1; \
    end \
  while (0)

  int a1[0:1];
  bit [31:0] a2[0:1];

  assign a1[0] = -10;
  assign a2[0] = 20;

  initial begin
    #0
    `check(a1[0], -10);
    `check(a2[0], 20);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
