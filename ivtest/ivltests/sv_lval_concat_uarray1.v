// Check that single element static unpacked array elements can be used in
// procedural l-value concatenations.

module test;

  reg failed;
  reg [7:0] a[0:0];
  reg [7:0] y;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    y = 8'h78;
    {a[0]} = y;

    `check(a[0], 8'h78);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
