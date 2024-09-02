module test;

// Check that the right hand side for a shift instruction is always treated as
// unsigned. Even if its a signed register, or a transformation thereof.

  reg failed = 1'b0;

  `define check(val, exp) \
    if ((val) !== (exp)) begin \
      $display("FAILED(%0d): `%s`, expected `%0d`, got `%0d`.", `__LINE__, \
               `"val`", (exp), (val), 4); \
      failed = 1'b1; \
    end

  reg signed [1:0] shift = 2'b10;

  initial begin
    `check(1 << shift, 4)
    `check(1 << shift[1:0], 4)
    `check(2 << shift[1], 4)
    `check(1 << $unsigned(shift), 4)
    `check(1 << $signed(shift), 4)
    `check(1 << {shift}, 4)

    `check(1 <<< shift, 4)
    `check(1 <<< shift[1:0], 4)
    `check(2 <<< shift[1], 4)
    `check(1 <<< $unsigned(shift), 4)
    `check(1 <<< $signed(shift), 4)
    `check(1 <<< {shift}, 4)

    `check(16 >> shift, 4)
    `check(16 >> shift[1:0], 4)
    `check(8 >> shift[1], 4)
    `check(16 >> $unsigned(shift), 4)
    `check(16 >> $signed(shift), 4)
    `check(16 >> {shift}, 4)

    `check(16 >>> shift, 4)
    `check(16 >>> shift[1:0], 4)
    `check(8 >>> shift[1], 4)
    `check(16 >>> $unsigned(shift), 4)
    `check(16 >>> $signed(shift), 4)
    `check(16 >>> {shift}, 4)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
