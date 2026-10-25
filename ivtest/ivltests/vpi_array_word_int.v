// Check VPI integer conversion to dynamic array and queue element widths.

module test;

  bit failed = 1'b0;
  reg [7:0] small_da [];
  reg [7:0] small_q [$];
  reg signed [64:0] wide_da [];
  reg signed [64:0] wide_q [$];
  bit signed [64:0] bits_da [];
  bit signed [64:0] bits_q [$];
  int int_da [];
  int int_q [$];

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  `define write(value, small, wide) \
    $put_array_int("test.small_da", 0, value); \
    `check(small_da[0], small); \
    $put_array_int("test.small_q", 0, value); \
    `check(small_q[0], small); \
    $put_array_int("test.wide_da", 0, value); \
    `check(wide_da[0], wide); \
    $put_array_int("test.wide_q", 0, value); \
    `check(wide_q[0], wide); \
    $put_array_int("test.bits_da", 0, value); \
    `check(bits_da[0], wide); \
    $put_array_int("test.bits_q", 0, value); \
    `check(bits_q[0], wide); \
    $put_array_int("test.int_da", 0, value); \
    `check(int_da[0], value); \
    $put_array_int("test.int_q", 0, value); \
    `check(int_q[0], value);

  initial begin
    small_da = new[1];
    small_q.push_back(8'b0);
    wide_da = new[1];
    wide_q.push_back(65'b0);
    bits_da = new[1];
    bits_q.push_back(65'b0);
    int_da = new[1];
    int_q.push_back(0);

    `write(32'sh00000000, 8'h00, 65'h00000000000000000);
    `write(32'sh00000001, 8'h01, 65'h00000000000000001);
    `write(32'shffffffff, 8'hff, 65'h1ffffffffffffffff);
    `write(32'sh80000000, 8'h00, 65'h1ffffffff80000000);
    `write(32'sh7fffffff, 8'hff, 65'h0000000007fffffff);
    `write(32'sh123456ab, 8'hab, 65'h000000000123456ab);

    if (!failed) $display("PASSED");
  end

endmodule
