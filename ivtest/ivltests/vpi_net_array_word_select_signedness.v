// Check that VPI conversions of packed selects of net array words are unsigned.

module test;
  reg [7:0] value;
  wire signed [7:0] words[3:4];
  wire signed [0:7] ascending[4:3];
  wire signed [15:8] shifted[-1:0];
  wire signed [-1:-8] negative[0:1];
  wire [7:0] unsigned_words[0:1];
  reg [8*32-1:0] text;
  reg failed;
  integer index, base;

  assign words[3] = value;
  assign words[4] = 8'h73;
  assign ascending[3] = value;
  assign shifted[-1] = value;
  assign negative[0] = value;
  assign unsigned_words[0] = value;

  `define check(fmt, val, exp) \
    $sformat(text, fmt, val); \
    if (text != exp) begin \
      $display("FAILED(%0d). '%s' expected %s, got %0s", `__LINE__, \
               `"val`", exp, text); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    value = 8'h83;
    index = 3;
    base = 4;
    #1;

    `check("%0d", words[3][7], "1");
    `check("%0d", words[3][7:4], "8");
    `check("%0d", words[3][4 +: 4], "8");
    `check("%0d", words[3][7 -: 4], "8");
    `check("%0d", words[3][7:0], "131");
    `check("%0d", words[3], "-125");
    `check("%0d", words[4][7:4], "7");

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
    `check("%0d", shifted[-1][15:12], "8");
    `check("%0d", negative[0][-1:-4], "8");
    `check("%0d", unsigned_words[0][7:4], "8");
    `check("%0d", unsigned_words[0], "131");
    `check("%.0f", words[3][7], "1");
    `check("%.0f", words[3][7:4], "8");

    if (!failed) $display("PASSED");
  end
endmodule
