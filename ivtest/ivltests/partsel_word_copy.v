module partsel_word_copy;
  reg [319:0] source;
  integer failed;

  task check;
    input [319:0] actual;
    input integer base;
    input integer width;
    input integer id;
    integer idx;
    begin
      for (idx = 0; idx < width; idx = idx + 1) begin
        if (actual[idx] !== source[base + idx]) begin
          $display("FAILED part-select word copy test %0d at bit %0d", id, idx);
          failed = failed + 1;
        end
      end
    end
  endtask

  initial begin
    failed = 0;
    source = {5{64'h0123456789abcdef}};
    source[31:28] = 4'hx;
    source[95:92] = 4'hz;
    source[203:196] = 8'b10xz_zx01;

    // Exercise contained selections around both 32- and 64-bit host-word
    // boundaries, with aligned and unaligned starting positions.
    check(source[0 +: 31], 0, 31, 1);
    check(source[32 +: 32], 32, 32, 2);
    check(source[31 +: 33], 31, 33, 3);
    check(source[64 +: 63], 64, 63, 4);
    check(source[64 +: 64], 64, 64, 5);
    check(source[63 +: 65], 63, 65, 6);
    check(source[64 +: 127], 64, 127, 7);
    check(source[64 +: 128], 64, 128, 8);
    check(source[63 +: 129], 63, 129, 9);
    check(source[64 +: 256], 64, 256, 10);

    // Preserve X padding when a selection overlaps or misses the source.
    check(source[280 +: 80], 280, 80, 11);
    check(source[400 +: 65], 400, 65, 12);
    check(source[-17 +: 65], -17, 65, 13);

    if (failed == 0)
      $display("PASSED");
  end
endmodule
