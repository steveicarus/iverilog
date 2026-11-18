// Check compound assignments to bit and part selects of array words.

module test;

  reg [7:0] words[0:1];
  integer word, base;
  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    words[0] = 8'h10;
    words[1] = 8'h20;
    words[0][7:4] += 1;
    `check(words[0], 8'h20);
    words[1][2] |= 1'b1;
    `check(words[1], 8'h24);

    word = 1;
    base = 4;
    words[word][base +: 4] += 1;
    `check(words[1], 8'h34);
    base = 2;
    words[word][base] ^= 1'b1;
    `check(words[1], 8'h30);
    `check(words[0], 8'h20);

    base = -2;
    words[word][base +: 4] |= 4'b1100;
    `check(words[1], 8'h33);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
