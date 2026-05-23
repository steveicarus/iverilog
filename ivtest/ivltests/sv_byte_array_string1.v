// Check that string literals can be assigned to one-dimensional byte arrays.

module test;

  bit failed = 1'b0;

  `define check(val, exp) do \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %02h, got %02h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  byte desc [3:0] = "AB\n";
  byte asc [0:3];
  byte unsized [4];

  assign asc = "AB\n";

  initial begin
    #1;

    `check(desc[3], 8'h41);
    `check(desc[2], 8'h42);
    `check(desc[1], 8'h0a);
    `check(desc[0], 8'h00);

    `check(asc[0], 8'h41);
    `check(asc[1], 8'h42);
    `check(asc[2], 8'h0a);
    `check(asc[3], 8'h00);

    unsized = "AB\n";
    `check(unsized[0], 8'h41);
    `check(unsized[1], 8'h42);
    `check(unsized[2], 8'h0a);
    `check(unsized[3], 8'h00);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
