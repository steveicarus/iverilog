// Check that VPI enum constant conversions preserve the base signedness.

module test;
  enum bit signed [0:0] {B1 = -1, B1_ZERO = 0} b1;
  enum bit signed [7:0] {B8 = -1, B8_MIN = -128, B8_POS = 127} b8;
  enum bit signed [15:0] {B16 = -1} b16;
  enum bit signed [30:0] {B31 = -1} b31;
  enum bit signed [31:0] {B32 = -1} b32;
  enum bit signed [63:0] {B64 = -1} b64;
  enum bit [7:0] {BU8 = 255} bu8;
  enum bit [31:0] {BU32 = 32'hffffffff} bu32;
  enum bit [63:0] {BU64 = 64'hffffffffffffffff} bu64;
  enum logic signed [0:0] {L1 = -1, L1_ZERO = 0} l1;
  enum logic signed [7:0] {L8 = -1, L8_MIN = -128, L8_POS = 127} l8;
  enum logic signed [30:0] {L31 = -1} l31;
  enum logic signed [31:0] {L32 = -1} l32;
  enum logic signed [64:0] {L65 = -1, L65_MIN = -65'sh10000000000000000} l65;
  enum logic [7:0] {LU8 = 255} lu8;
  enum logic [31:0] {LU32 = 32'hffffffff} lu32;
  enum logic [64:0] {LU65 = 65'h10000000000000000} lu65;
  enum logic signed [7:0] {
    NEG_XZ = 8'b1x0z0011, POS_XZ = 8'b0x0z0011,
    SIGN_X = 8'bx1000001, SIGN_Z = 8'bz1000001
  } xz;
  bit failed = 1'b0;

  `define check_int(name, exp) \
    if ($enum_int(`"name`") !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"name`", exp, $enum_int(`"name`")); \
      failed = 1'b1; \
    end

  `define check_string(name, exp) \
    if ($enum_decimal(`"name`") != exp) begin \
      $display("FAILED(%0d). '%s' expected %s, got %s", `__LINE__, \
               `"name`", exp, $enum_decimal(`"name`")); \
      failed = 1'b1; \
    end

  `define check_real(name, exp) \
    if ($enum_real(`"name`") != exp) begin \
      $display("FAILED(%0d). '%s' expected %g, got %g", `__LINE__, \
               `"name`", exp, $enum_real(`"name`")); \
      failed = 1'b1; \
    end

  initial begin
    `check_int(B1, -1);
    `check_int(B1_ZERO, 0);
    `check_int(B8, -1);
    `check_int(B8_MIN, -128);
    `check_int(B8_POS, 127);
    `check_int(B16, -1);
    `check_int(B31, -1);
    `check_int(B32, -1);
    `check_int(B64, -1);
    `check_int(BU8, 255);
    `check_int(BU32, -1);
    `check_int(BU64, -1);
    `check_int(L1, -1);
    `check_int(L1_ZERO, 0);
    `check_int(L8, -1);
    `check_int(L8_MIN, -128);
    `check_int(L8_POS, 127);
    `check_int(L31, -1);
    `check_int(L32, -1);
    `check_int(L65, -1);
    `check_int(L65_MIN, 0);
    `check_int(LU8, 255);
    `check_int(LU32, -1);
    `check_int(LU65, 0);
    `check_int(NEG_XZ, -125);
    `check_int(POS_XZ, 3);
    `check_int(SIGN_X, 65);
    `check_int(SIGN_Z, 65);

    `check_string(L1, "-1");
    `check_string(L1_ZERO, "0");
    `check_string(L8, "-1");
    `check_string(L8_MIN, "-128");
    `check_string(L8_POS, "127");
    `check_string(L31, "-1");
    `check_string(L32, "-1");
    `check_string(L65, "-1");
    `check_string(L65_MIN, "-18446744073709551616");
    `check_string(LU8, "255");
    `check_string(LU32, "4294967295");
    `check_string(LU65, "18446744073709551616");

    `check_real(L1, -1.0);
    `check_real(L1_ZERO, 0.0);
    `check_real(L8, -1.0);
    `check_real(L8_MIN, -128.0);
    `check_real(L8_POS, 127.0);
    `check_real(L31, -1.0);
    `check_real(L32, -1.0);
    `check_real(L65, -1.0);
    `check_real(L65_MIN, -18446744073709551616.0);
    `check_real(LU8, 255.0);
    `check_real(LU32, 4294967295.0);
    `check_real(LU65, 18446744073709551616.0);

    if (!failed) $display("PASSED");
  end
endmodule
