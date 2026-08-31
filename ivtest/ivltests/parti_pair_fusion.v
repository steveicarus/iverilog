module parti_pair_fusion;
  localparam [8:0] UBASE = 9'd64;

  reg [319:0] source_a;
  reg [319:0] source_b;
  reg [31:0] source_word;
  reg [63:0] source_native;
  reg [1:0] actual2;
  reg [31:0] actual32;
  reg [63:0] actual64;
  reg [65:0] actual66;
  reg [128:0] actual129;
  reg [129:0] actual130;
  reg [257:0] actual258;
  integer failed;

  // Build the reference result one bit at a time. This deliberately avoids
  // the adjacent constant part-select pattern exercised by the implementation.
  function automatic [257:0] reference_concat;
    input [319:0] left_source;
    input [319:0] right_source;
    input integer left_base;
    input integer left_width;
    input integer right_base;
    input integer right_width;
    integer idx;
    begin
      reference_concat = 258'b0;
      for (idx = 0; idx < right_width; idx = idx + 1)
        reference_concat[idx] = right_source[right_base + idx];
      for (idx = 0; idx < left_width; idx = idx + 1)
        reference_concat[right_width + idx] = left_source[left_base + idx];
    end
  endfunction

  function automatic [128:0] reference_xor;
    input [319:0] left_source;
    input [319:0] right_source;
    input integer left_base;
    input integer right_base;
    input integer width;
    integer idx;
    begin
      reference_xor = 129'b0;
      for (idx = 0; idx < width; idx = idx + 1)
        reference_xor[idx] = left_source[left_base + idx]
                           ^ right_source[right_base + idx];
    end
  endfunction

  task check;
    input [257:0] got;
    input [257:0] want;
    input integer id;
    begin
      if (got !== want) begin
        $display("FAILED part-select pair test %0d", id);
        failed = failed + 1;
      end
    end
  endtask

  initial begin
    failed = 0;
    source_a = {5{64'h0123456789abcdef}};
    source_b = {5{64'hfedcba9876543210}};
    source_a[5] = 1'bx;
    source_a[145] = 1'bz;
    source_b[75] = 1'bz;
    source_b[200] = 1'bx;
    source_word = 32'h89abcdef;
    source_word[5] = 1'bx;
    source_word[27] = 1'bz;
    source_native = 64'h0123456789abcdef;
    source_native[9] = 1'bx;
    source_native[54] = 1'bz;

    // Differently sized selected signals consumed by concatenation.
    actual64 = {source_a[3 +: 31], source_b[69 +: 33]};
    check(actual64, reference_concat(source_a, source_b, 3, 31, 69, 33), 1);

    // Same-source slices and a native-word boundary. UBASE exercises an
    // explicitly sized constant base.
    actual129 = {source_a[0 +: 64], source_a[UBASE +: 65]};
    check(actual129, reference_concat(source_a, source_a, 0, 64,
                                      UBASE, 65), 2);

    // Wide slices exercise heap-backed vector storage.
    actual258 = {source_a[1 +: 129], source_b[128 +: 129]};
    check(actual258, reference_concat(source_a, source_b, 1, 129,
                                      128, 129), 3);

    // A pair consumed by a regular binary operation still shares one
    // interpreter dispatch while preserving stack order.
    actual129 = source_a[64 +: 65] ^ source_b[127 +: 65];
    check(actual129, reference_xor(source_a, source_b, 64, 127, 65), 4);

    // Positive out-of-range selects retain %parti's X padding.
    actual66 = {source_a[300 +: 33], source_b[310 +: 33]};
    check(actual66, reference_concat(source_a, source_b, 300, 33,
                                     310, 33), 5);

    // Forced bits use the filtered signal-value path.
    force source_a[70] = 1'bx;
    force source_b[140] = 1'bz;
    actual130 = {source_a[64 +: 65], source_b[128 +: 65]};
    check(actual130, reference_concat(source_a, source_b, 64, 65,
                                      128, 65), 6);
    release source_a[70];
    release source_b[140];

    // A negative signed base on either side deliberately stays on the
    // unfused path and must preserve X padding and stack order.
    actual130 = {source_a[-5 +: 65], source_b[8 +: 65]};
    check(actual130, reference_concat(source_a, source_b, -5, 65,
                                      8, 65), 7);
    actual130 = {source_a[8 +: 65], source_b[-5 +: 65]};
    check(actual130, reference_concat(source_a, source_b, 8, 65,
                                      -5, 65), 8);

    // Tiny and subword pairs cover the inline-vector cases.
    actual2 = {source_a[5 +: 1], source_b[75 +: 1]};
    check(actual2, reference_concat(source_a, source_b, 5, 1, 75, 1), 9);
    actual64 = {source_a[31 +: 32], source_b[63 +: 32]};
    check(actual64, reference_concat(source_a, source_b, 31, 32,
                                     63, 32), 10);

    // Same-source subword pairs can be assembled from one signal read.
    actual32 = {source_word[0 +: 7], source_word[7 +: 25]};
    check(actual32, reference_concat(source_word, source_word, 0, 7,
                                     7, 25), 11);

    // The direct path must still observe forced values.
    force source_word[20] = 1'bx;
    actual32 = {source_word[16 +: 16], source_word[0 +: 16]};
    check(actual32, reference_concat(source_word, source_word, 16, 16,
                                     0, 16), 12);
    release source_word[20];

    // Fill a native word on 64-bit hosts; this remains a generic-path
    // correctness check on 32-bit hosts.
    actual64 = {source_native[0 +: 32], source_native[32 +: 32]};
    check(actual64, reference_concat(source_native, source_native, 0, 32,
                                     32, 32), 13);

    if (failed == 0)
      $display("PASSED");
  end
endmodule
