// Check that string literals can be assigned to nested byte arrays using
// assignment patterns.

module test;

  bit failed = 1'b0;

  `define check(val, exp) do \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %02h, got %02h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  byte desc [0:1][3:0] = '{"AB\n", "CD\t"};
  byte asc [1:0][0:3];
  byte unsized [2][4];

  assign asc = '{"AB\n", "CD\t"};

  initial begin
    #1;

    `check(desc[0][3], 8'h41);
    `check(desc[0][2], 8'h42);
    `check(desc[0][1], 8'h0a);
    `check(desc[0][0], 8'h00);

    `check(desc[1][3], 8'h43);
    `check(desc[1][2], 8'h44);
    `check(desc[1][1], 8'h09);
    `check(desc[1][0], 8'h00);

    `check(asc[0][0], 8'h43);
    `check(asc[0][1], 8'h44);
    `check(asc[0][2], 8'h09);
    `check(asc[0][3], 8'h00);

    `check(asc[1][0], 8'h41);
    `check(asc[1][1], 8'h42);
    `check(asc[1][2], 8'h0a);
    `check(asc[1][3], 8'h00);

    unsized = '{"AB\n", "CD\t"};
    `check(unsized[0][0], 8'h41);
    `check(unsized[0][1], 8'h42);
    `check(unsized[0][2], 8'h0a);
    `check(unsized[0][3], 8'h00);

    `check(unsized[1][0], 8'h43);
    `check(unsized[1][1], 8'h44);
    `check(unsized[1][2], 8'h09);
    `check(unsized[1][3], 8'h00);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
