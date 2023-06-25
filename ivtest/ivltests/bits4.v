// Check that passing a array identifiers and array slices to $bits works as expected

module test;

  bit failed = 1'b0;

  `define check(expr, value) do begin \
    if ($bits(expr) !== value) begin \
      $display("FAILED(%d): $bits(", `"expr`", ") is %0d", `__LINE__, $bits(expr), " expected %0d", value); \
      failed = 1'b1; \
    end \
  end while (0)

  typedef bit T[3:0];

  T x;
  byte y[7:0][2:0];

  initial begin
    integer i;
    i = 4;

    `check(x, 4);
    `check(y, $bits(byte) * 3 * 8);
    `check(y[0], $bits(byte) * 3);
    `check(y[1:0], $bits(byte) * 3 * 2);
    `check(y[1+:3], $bits(byte) * 3 * 3);
    `check(y[4-:4], $bits(byte) * 3 * 4);
    `check(y[i-:2], $bits(byte) * 3 * 2);
    `check(y[i+:2], $bits(byte) * 3 * 2);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
