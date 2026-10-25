// Check scalar VPI writes to dynamic array and queue elements.

module test;

  bit failed = 1'b0;
  reg da [];
  reg q [$];
  reg [64:0] wide_da [];
  reg [64:0] wide_q [$];
  bit bits_da [];
  bit bits_q [$];

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  `define write(value) \
    $put_array_scalar("test.da", 0, value); \
    `check(da[0], value); \
    $put_array_scalar("test.q", 0, value); \
    `check(q[0], value); \
    $put_array_scalar("test.wide_da", 0, value); \
    `check(wide_da[0], {64'b0, value}); \
    $put_array_scalar("test.wide_q", 0, value); \
    `check(wide_q[0], {64'b0, value}); \
    $put_array_scalar("test.bits_da", 0, value); \
    `check(bits_da[0], (value === 1'b1)); \
    $put_array_scalar("test.bits_q", 0, value); \
    `check(bits_q[0], (value === 1'b1));

  initial begin
    da = new[1];
    wide_da = new[1];
    bits_da = new[1];
    q.push_back(1'b0);
    wide_q.push_back(65'b0);
    bits_q.push_back(1'b0);

    `write(1'b1);
    `write(1'b0);
    `write(1'bx);
    `write(1'bz);
    `write(1'b1);

    if (!failed) $display("PASSED");
  end

endmodule
