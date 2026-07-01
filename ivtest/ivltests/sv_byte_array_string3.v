// Check that string literals shorter than the target byte array are padded
// with null bytes.

module test;

  bit failed = 1'b0;

  `define check(val, exp) do \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %02h, got %02h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  byte desc [3:0] = "AB";
  byte asc [0:3];
  byte unsized [4];

  assign asc = "AB";

  initial begin
    #1;

    `check(desc[3], 8'h41);
    `check(desc[2], 8'h42);
    `check(desc[1], 8'h00);
    `check(desc[0], 8'h00);

    `check(asc[0], 8'h41);
    `check(asc[1], 8'h42);
    `check(asc[2], 8'h00);
    `check(asc[3], 8'h00);

    unsized = "AB";
    `check(unsized[0], 8'h41);
    `check(unsized[1], 8'h42);
    `check(unsized[2], 8'h00);
    `check(unsized[3], 8'h00);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
