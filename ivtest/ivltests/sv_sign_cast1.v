// Check that sign casts have the expected results when the value gets width
// extended.

module test;
  bit failed = 1'b0;
  reg [7:0] val;
  reg signed [7:0] sval;

  `define check(val, exp) \
    if (exp !== val) begin \
      $display("FAILED(%0d). Got %b, expected %b.", `__LINE__, val, exp); \
      failed = 1'b1; \
    end

  initial begin
    // An unsized number has an implicit width of integer width.
    val = unsigned'(-4);
    `check(val, 8'hfc);

    val = unsigned'(-4'sd4);
    `check(val, 8'h0c);

    sval = signed'(4'hc);
    `check(sval, -4);

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
