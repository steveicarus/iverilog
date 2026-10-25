// Check width and signedness of constant binary NAND/NOR expressions.

module test;

  localparam [7:0] UNSIGNED_NAND = 4'h5 ~& 4'h3;
  localparam [7:0] UNSIGNED_NOR = 4'h5 ~| 4'h3;
  localparam [7:0] SIGNED_NAND = (4'sh7 ~& 4'sh7) >>> 1;
  localparam [7:0] SIGNED_NOR = (4'sh1 ~| 4'sh2) >>> 1;
  localparam [7:0] MIXED_NAND = (4'sh7 ~& 4'h7) >>> 1;
  localparam [7:0] MIXED_NOR = (4'sh1 ~| 4'h2) >>> 1;
  localparam [64:0] WIDE_NAND = 65'd0 ~& 65'd0;
  localparam [64:0] WIDE_NOR = 65'd0 ~| 65'd0;

  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    `check(UNSIGNED_NAND, 8'hfe);
    `check(UNSIGNED_NOR, 8'hf8);
    `check(SIGNED_NAND, 8'hfc);
    `check(SIGNED_NOR, 8'hfe);
    `check(MIXED_NAND, 8'h7c);
    `check(MIXED_NOR, 8'h7e);
    `check(WIDE_NAND, {65{1'b1}});
    `check(WIDE_NOR, {65{1'b1}});

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
