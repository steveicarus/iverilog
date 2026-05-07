// Check that string literals longer than the target byte array are truncated.

module test;

  bit failed = 1'b0;

  `define check(val, exp) do \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %02h, got %02h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  byte desc [1:0] = "ABC";
  byte asc [0:1];
  byte unsized [2];

  assign asc = "ABC";

  initial begin
    #1;

    `check(desc[1], 8'h41);
    `check(desc[0], 8'h42);

    `check(asc[0], 8'h41);
    `check(asc[1], 8'h42);

    unsized = "ABCD";
    `check(unsized[0], 8'h41);
    `check(unsized[1], 8'h42);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
