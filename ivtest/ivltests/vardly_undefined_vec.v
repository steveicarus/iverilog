module test;
  // Test that intra-assignment delay values of 'z and 'x get treated as a zero
  // delay. Check this for different types of assignments. The assignment should
  // not be skipped.

  reg failed = 1'b0;

  `define check(expr, val) \
    if (expr !== val) begin \
      $display("FAILED(%0d): `%s`, expected %0x, got %0x", `__LINE__, `"expr`", val, expr); \
      failed = 1'b1; \
    end


  integer delay_x = 32'hx;
  wire [31:0] delay_z;

  reg [31:0] x;
  reg [31:0] a[0:1];
  integer i = 0, j = 0;

  `define test(var) \
    // Non-blocking \
    var = 0; \
    var <= #delay_x 1; \
    #1 `check(var, 1) \
    var = 0; \
    var <= #delay_z 1; \
    #1 `check(var, 1) \
    // blocking \
    var = 0; \
    var = #delay_x 1; \
    `check(var, 1) \
    var = 0; \
    var = #delay_z 1; \
    `check(var, 1)

  initial begin
    `test(x)
    `test(x[0])
    `test(x[i])
    `test(a[0])
    `test(a[0][0])
    `test(a[0][j])
    `test(a[i])
    `test(a[i][0])
    `test(a[i][j])

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
