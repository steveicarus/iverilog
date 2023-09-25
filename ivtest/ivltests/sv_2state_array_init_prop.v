// Check that the initial value of a 2-state array is properly propagated

module test;

  bit failed = 1'b0;

  `define check(expr, val) \
    if (expr !== val) begin \
      $display("FAILED(%0d): `%s`, expected %0d, got %0d", `__LINE__, `"expr`", val, expr); \
      failed = 1'b1; \
    end

  bit [1:0] a[2];
  integer i = 0;

  wire [1:0] x = a[0];
  wire [1:0] y = a[i];
  wire [2:0] z = {1'b1, a[0]};
  wire w = a[0][0];

  initial begin
     #1;
    `check(a[0], 2'b00);
    `check(x, 2'b00);
    `check(y, 2'b00);
    `check(z, 3'b100);
    `check(w, 1'b0);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
