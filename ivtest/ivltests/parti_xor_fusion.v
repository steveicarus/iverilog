module parti_xor_fusion;
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
  reg result1;
  reg [30:0] result31;
  reg [31:0] result32;
  reg [32:0] result33;
  reg [63:0] result64;
  reg [64:0] result65;
  reg [126:0] result127;
  reg [256:0] result257;
  integer failed;

  function automatic [256:0] reference_xor;
    input [256:0] lhs;
    input integer base;
    input integer width;
    integer idx;
    begin
      reference_xor = 257'b0;
      for (idx = 0; idx < width; idx = idx + 1)
        reference_xor[idx] = lhs[idx] ^ source[base + idx];
    end
  endfunction

  task check;
    input [256:0] actual;
    input [256:0] expected;
    input integer id;
    begin
      if (actual !== expected) begin
        $display("FAILED part-select XOR test %0d", id);
        failed = failed + 1;
      end
    end
  endtask

  initial begin
    failed = 0;
    source = {64'h0123456789abcdef, 64'hfedcba9876543210,
              64'h0f0f0f0f0f0f0f0f, 64'hf0f0f0f0f0f0f0f0};
    source[3] = 1'bx;
    source[68] = 1'bz;
    source[140] = 1'bx;

    lhs1 = 1'b1;
    lhs31 = 31'h55aa55aa;
    lhs32 = 32'h89abcdef;
    lhs33 = 33'h1_12345678;
    lhs64 = 64'h0123456789abcdef;
    lhs65 = 65'h1_13579bdf_2468ace0;
    lhs127 = 127'h71234567_89abcdef_02468ace_13579bdf;
    lhs257 = {1'b1, {4{64'h0123456789abcdef}}};
    lhs65[7] = 1'bx;
    lhs127[80] = 1'bz;

    result1 = lhs1 ^ source[0 +: 1];
    check(result1, reference_xor(lhs1, 0, 1), 1);

    result31 = lhs31 ^ source[7 +: 31];
    check(result31, reference_xor(lhs31, 7, 31), 2);

    result32 = lhs32 ^ source[31 +: 32];
    check(result32, reference_xor(lhs32, 31, 32), 3);

    result33 = lhs33 ^ source[63 +: 33];
    check(result33, reference_xor(lhs33, 63, 33), 4);

    result64 = lhs64 ^ source[64 +: 64];
    check(result64, reference_xor(lhs64, 64, 64), 5);

    // Exercise an explicitly sized base at the native-word boundary.
    result65 = lhs65 ^ source[UBASE +: 65];
    check(result65, reference_xor(lhs65, UBASE, 65), 6);

    result127 = lhs127 ^ source[63 +: 127];
    check(result127, reference_xor(lhs127, 63, 127), 7);

    // Cross the source boundary by one bit and exercise a wide result.
    result257 = lhs257 ^ source[0 +: 257];
    check(result257, reference_xor(lhs257, 0, 257), 8);

    // Partially and wholly out-of-range positive bases remain fuseable and
    // must preserve the X padding of %parti.
    result65 = lhs65 ^ source[240 +: 65];
    check(result65, reference_xor(lhs65, 240, 65), 9);
    result65 = lhs65 ^ source[300 +: 65];
    check(result65, reference_xor(lhs65, 300, 65), 10);

    // Negative signed bases use the unfused fallback.
    result65 = lhs65 ^ source[-5 +: 65];
    check(result65, reference_xor(lhs65, -5, 65), 11);

    // Forced bits take the filtered signal-value path.
    force source[70] = 1'bx;
    result65 = lhs65 ^ source[64 +: 65];
    check(result65, reference_xor(lhs65, 64, 65), 12);
    release source[70];

    if (failed == 0)
      $display("PASSED");
  end
endmodule
