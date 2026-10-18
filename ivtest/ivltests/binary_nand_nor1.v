// Check constant binary NAND/NOR for all pairs of four-state bit values.

module test;

  localparam [15:0] LEFT = 16'b0000_1111_xxxx_zzzz;
  localparam [15:0] RIGHT = 16'b01xz_01xz_01xz_01xz;
  localparam [15:0] NAND = LEFT ~& RIGHT;
  localparam [15:0] NOR = LEFT ~| RIGHT;

  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    `check(NAND, 16'b1111_10xx_1xxx_1xxx);
    `check(NOR, 16'b10xx_0000_x0xx_x0xx);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
