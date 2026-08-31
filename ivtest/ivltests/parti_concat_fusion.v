module parti_concat_fusion;
  localparam [8:0] UBASE = 9'd65;

  reg [255:0] source;
  reg lhs1;
  reg [6:0] lhs7;
  reg [30:0] lhs31;
  reg [31:0] lhs32;
  reg [63:0] lhs64;
  reg [64:0] lhs65;
  reg [256:0] lhs257;
  reg [1:0] result2;
  reg [37:0] result38;
  reg [63:0] result64;
  reg [64:0] result65;
  reg [127:0] result128;
  reg [191:0] result192;
  reg [513:0] result514;
  integer failed;

  function automatic [513:0] reference_concat;
    input [256:0] lhs;
    input integer lhs_width;
    input integer base;
    input integer select_width;
    integer idx;
    begin
      reference_concat = 514'b0;
      for (idx = 0; idx < select_width; idx = idx + 1)
        reference_concat[idx] = source[base + idx];
      for (idx = 0; idx < lhs_width; idx = idx + 1)
        reference_concat[select_width + idx] = lhs[idx];
    end
  endfunction

  task check;
    input [513:0] actual;
    input [513:0] expected;
    input integer id;
    begin
      if (actual !== expected) begin
        $display("FAILED part-select concat test %0d", id);
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
    lhs7 = 7'h55;
    lhs31 = 31'h55aa55aa;
    lhs32 = 32'h89abcdef;
    lhs64 = 64'h0123456789abcdef;
    lhs65 = 65'h1_13579bdf_2468ace0;
    lhs257 = {1'b1, {4{64'h0123456789abcdef}}};
    lhs65[7] = 1'bx;
    lhs257[180] = 1'bz;

    result2 = {lhs1, source[0 +: 1]};
    check(result2, reference_concat(lhs1, 1, 0, 1), 1);

    result38 = {lhs7, source[7 +: 31]};
    check(result38, reference_concat(lhs7, 7, 7, 31), 2);

    result64 = {lhs31, source[31 +: 33]};
    check(result64, reference_concat(lhs31, 31, 31, 33), 3);

    result65 = {lhs32, source[63 +: 33]};
    check(result65, reference_concat(lhs32, 32, 63, 33), 4);

    result128 = {lhs64, source[64 +: 64]};
    check(result128, reference_concat(lhs64, 64, 64, 64), 5);

    // Exercise an explicitly sized base and a wide destination.
    result192 = {lhs65, source[UBASE +: 127]};
    check(result192, reference_concat(lhs65, 65, UBASE, 127), 6);

    result514 = {lhs257, source[0 +: 257]};
    check(result514, reference_concat(lhs257, 257, 0, 257), 7);

    // Positive out-of-range selections remain fuseable and retain X padding.
    result65 = {lhs32, source[240 +: 33]};
    check(result65, reference_concat(lhs32, 32, 240, 33), 8);
    result65 = {lhs32, source[300 +: 33]};
    check(result65, reference_concat(lhs32, 32, 300, 33), 9);

    // Negative bases use the unfused fallback.
    result65 = {lhs32, source[-5 +: 33]};
    check(result65, reference_concat(lhs32, 32, -5, 33), 10);

    // Forced bits take the filtered signal-value path.
    force source[70] = 1'bx;
    result128 = {lhs64, source[64 +: 64]};
    check(result128, reference_concat(lhs64, 64, 64, 64), 11);
    release source[70];

    if (failed == 0)
      $display("PASSED");
  end
endmodule
