// Check that VPI conversions of packed selects of array words are unsigned.

module test;
  reg signed [7:0] words[3:4];
  reg signed [0:7] ascending[4:3];
  reg signed [15:8] shifted[-1:0];
  reg signed [-1:-8] negative[0:1];
  reg signed [71:0] wide[0:1];
  reg [7:0] unsigned_words[0:1];
  reg signed [7:0] ordinary;
  reg [8*32-1:0] text;
  reg failed;
  integer index, base, fd;

  `define check(fmt, val, exp) \
    $sformat(text, fmt, val); \
    if (text != exp) begin \
      $display("FAILED(%0d). '%s' expected %s, got %0s", `__LINE__, \
               `"val`", exp, text); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    words[3] = 8'h83;
    words[4] = 8'h73;
    ascending[3] = words[3];
    shifted[-1] = words[3];
    negative[0] = words[3];
    wide[0] = 72'h080000000000000000;
    unsigned_words[0] = words[3];
    ordinary = words[3];
    index = 3;
    base = 4;

    `check("%0d", words[3][7], "1");
    `check("%0d", words[3][7:4], "8");
    `check("%0d", words[3][4 +: 4], "8");
    `check("%0d", words[3][7 -: 4], "8");
    `check("%0d", words[3][7:0], "131");
    `check("%0d", words[3], "-125");
    `check("%0d", words[4][7:4], "7");

    `check("%0d", words[index][7], "1");
    `check("%0d", words[index][7:4], "8");
    `check("%0d", words[index][7:0], "131");
    `check("%0d", words[index], "-125");
    `check("%0d", words[3][base +: 4], "8");
    `check("%0d", words[index][base +: 4], "8");

    `check("%0d", ascending[3][0], "1");
    `check("%0d", ascending[3][0:3], "8");
    `check("%0d", ascending[3][3 -: 4], "8");
    `check("%0d", ascending[3][0:7], "131");
    `check("%0d", ascending[3], "-125");

    `check("%0d", shifted[-1][15], "1");
    `check("%0d", shifted[-1][15:12], "8");
    `check("%0d", shifted[-1][12 +: 4], "8");
    `check("%0d", negative[0][-1], "1");
    `check("%0d", negative[0][-1:-4], "8");
    `check("%0d", negative[0][-4 +: 4], "8");
    `check("%0d", wide[0][67:4], "9223372036854775808");

    `check("%0d", unsigned_words[0][7:4], "8");
    `check("%0d", unsigned_words[0], "131");
    `check("%0d", ordinary[7:4], "8");
    `check("%0d", $signed(words[3][7:4]), "-8");

    `check("%.0f", words[3][7], "1");
    `check("%.0f", words[3][7:4], "8");
    `check("%.0f", ascending[3][0:3], "8");
    `check("%.0f", wide[0][67:4], "9223372036854775808");

    // $fputc requests an integer value instead of a formatted string.
    fd = $fopen("work/vpi_array_word_select_signedness.txt", "wb");
    `check("%0d", $fputc(words[3][7], fd), "0");
    `check("%0d", $fputc(words[3][7:4], fd), "0");
    $fclose(fd);
    fd = $fopen("work/vpi_array_word_select_signedness.txt", "rb");
    `check("%0d", $fgetc(fd), "1");
    `check("%0d", $fgetc(fd), "8");
    $fclose(fd);

    words[3] = 8'hxx;
    `check("%0d", words[3][7:4], "x");
    words[3] = 8'hzz;
    `check("%0d", words[3][7:4], "z");

    if (!failed) $display("PASSED");
  end
endmodule
