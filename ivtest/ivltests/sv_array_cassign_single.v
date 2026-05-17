// Check that continuous array assignment to single element unpacked arrays is
// supported.

module test;

  reg failed;
  wire [31:0] a[0:0];
  reg [31:0] b[0:0];

  assign a = b;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    b[0] = 32'd42;
    #1;
    `check(a[0], 32'd42);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
