module parti_add_fusion;
  localparam [8:0] UBASE = 9'd65;

  reg [255:0] source;
  reg lhs1;
  reg [30:0] lhs31;
  reg [31:0] lhs32;
  reg [32:0] lhs33;
  reg [63:0] lhs64;
  reg [64:0] lhs65;
  reg [126:0] lhs127;
  reg [256:0] lhs257;
  reg rhs1;
  reg [30:0] rhs31;
  reg [31:0] rhs32;
  reg [32:0] rhs33;
  reg [63:0] rhs64;
  reg [64:0] rhs65;
  reg [126:0] rhs127;
  reg [256:0] rhs257;
  reg expected1;
  reg [30:0] expected31;
  reg [31:0] expected32;
  reg [32:0] expected33;
  reg [63:0] expected64;
  reg [64:0] expected65;
  reg [126:0] expected127;
  reg [256:0] expected257;
  reg result1;
  reg [30:0] result31;
  reg [31:0] result32;
  reg [32:0] result33;
  reg [63:0] result64;
  reg [64:0] result65;
  reg [126:0] result127;
  reg [256:0] result257;
  integer failed;

  task check;
    input [256:0] actual;
    input [256:0] expected;
    input integer id;
    begin
      if (actual !== expected) begin
        $display("FAILED part-select add test %0d", id);
        failed = failed + 1;
      end
    end
  endtask

  initial begin
    failed = 0;
    source = {64'h0123456789abcdef, 64'hfedcba9876543210,
              64'h0f0f0f0f0f0f0f0f, 64'hf0f0f0f0f0f0f0f0};
    lhs1 = 1'b1;
    lhs31 = 31'h55aa55aa;
    lhs32 = 32'h89abcdef;
    lhs33 = 33'h1_12345678;
    lhs64 = 64'h0123456789abcdef;
    lhs65 = 65'h1_13579bdf_2468ace0;
    lhs127 = 127'h71234567_89abcdef_02468ace_13579bdf;
    lhs257 = {1'b1, {4{64'h0123456789abcdef}}};

    // Materialize each reference operand in a separate statement so the
    // expected add cannot use the load/part-select/add fusion under test.
    rhs1 = source[0 +: 1];
    expected1 = lhs1 + rhs1;
    result1 = lhs1 + source[0 +: 1];
    check(result1, expected1, 1);

    rhs31 = source[7 +: 31];
    expected31 = lhs31 + rhs31;
    result31 = lhs31 + source[7 +: 31];
    check(result31, expected31, 2);

    rhs32 = source[31 +: 32];
    expected32 = lhs32 + rhs32;
    result32 = lhs32 + source[31 +: 32];
    check(result32, expected32, 3);

    rhs33 = source[63 +: 33];
    expected33 = lhs33 + rhs33;
    result33 = lhs33 + source[63 +: 33];
    check(result33, expected33, 4);

    rhs64 = source[64 +: 64];
    expected64 = lhs64 + rhs64;
    result64 = lhs64 + source[64 +: 64];
    check(result64, expected64, 5);

    // Exercise an explicitly sized base at the native-word boundary.
    rhs65 = source[UBASE +: 65];
    expected65 = lhs65 + rhs65;
    result65 = lhs65 + source[UBASE +: 65];
    check(result65, expected65, 6);

    rhs127 = source[63 +: 127];
    expected127 = lhs127 + rhs127;
    result127 = lhs127 + source[63 +: 127];
    check(result127, expected127, 7);

    // Cross the source boundary by one bit and exercise a wide result.
    rhs257 = source[0 +: 257];
    expected257 = lhs257 + rhs257;
    result257 = lhs257 + source[0 +: 257];
    check(result257, expected257, 8);

    // X/Z inputs and positive out-of-range selections produce all X.
    source[68] = 1'bz;
    rhs65 = source[64 +: 65];
    expected65 = lhs65 + rhs65;
    result65 = lhs65 + source[64 +: 65];
    check(result65, expected65, 9);
    source[68] = 1'b0;

    rhs65 = source[240 +: 65];
    expected65 = lhs65 + rhs65;
    result65 = lhs65 + source[240 +: 65];
    check(result65, expected65, 10);
    rhs65 = source[300 +: 65];
    expected65 = lhs65 + rhs65;
    result65 = lhs65 + source[300 +: 65];
    check(result65, expected65, 11);

    // Negative signed bases use the unfused fallback.
    rhs65 = source[-5 +: 65];
    expected65 = lhs65 + rhs65;
    result65 = lhs65 + source[-5 +: 65];
    check(result65, expected65, 12);

    // Forced bits take the filtered signal-value path.
    force source[70] = 1'bx;
    rhs65 = source[64 +: 65];
    expected65 = lhs65 + rhs65;
    result65 = lhs65 + source[64 +: 65];
    check(result65, expected65, 13);
    release source[70];

    if (failed == 0)
      $display("PASSED");
  end
endmodule
