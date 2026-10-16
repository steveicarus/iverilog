// Check that real formatting preserves the signedness of integral expressions.

module test;
  reg signed scalar;
  reg signed [7:0] byte_value;
  integer value;
  reg signed [32:0] value33;
  reg signed [63:0] word;
  reg signed [64:0] wide;
  reg signed [2047:0] extended;
  reg [7:0] unsigned_value;
  reg [8*32-1:0] text;
  reg failed;

  `define check(val, exp) \
    $sformat(text, "%.0f", val); \
    if (text != exp) begin \
      $display("FAILED(%0d). '%s' expected %s, got %0s", `__LINE__, \
               `"val`", exp, text); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    scalar = 1'b1;
    byte_value = -3;
    value = -1;
    value33 = -1;
    word = -1;
    wide = -1;
    extended = -1;
    unsigned_value = 8'hfd;

    `check(value + 0, "-1");
    `check(scalar ^ 1'sb0, "-1");
    `check(byte_value ^ 8'sb0, "-3");
    `check(value33 ^ 33'sb0, "-1");
    `check(word ^ 64'sb0, "-1");
    `check(wide ^ 65'sb0, "-1");
    `check(extended ^ 2048'sb0, "-1");
    `check(unsigned_value ^ 8'b0, "253");
    `check($signed(unsigned_value), "-3");
    `check($unsigned(byte_value), "253");
    `check(byte_value ^ 8'b0, "253");

    scalar = 1'b0;
    byte_value = -128;
    value = 32'sh80000000;
    value33 = 33'sh100000000;
    word = 64'sh8000000000000000;
    wide = 65'sh10000000000000000;
    extended = -3;

    `check(scalar ^ 1'sb0, "0");
    `check(byte_value ^ 8'sb0, "-128");
    `check(value + 0, "-2147483648");
    `check(value33 ^ 33'sb0, "-4294967296");
    `check(word ^ 64'sb0, "-9223372036854775808");
    `check(wide ^ 65'sb0, "-18446744073709551616");
    `check(extended ^ 2048'sb0, "-3");
    `check(word ^ 64'b0, "9223372036854775808");
    `check(wide ^ 65'b0, "18446744073709551616");

    byte_value = 127;
    value = 0;
    wide = 1;
    `check(byte_value ^ 8'sb0, "127");
    `check(value + 0, "0");
    `check(wide ^ 65'sb0, "1");

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
