// Check saved and unknown indices in compound assignments to array part selects.

module test;

  reg [7:0] words[0:1];
  integer word, base, word_calls, base_calls;
  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  function integer choose_word(input integer unused);
    word_calls = word_calls+1;
    choose_word = word;
  endfunction

  function integer choose_base(input integer unused);
    base_calls = base_calls+1;
    choose_base = base;
  endfunction

  function [3:0] rhs(input integer unused);
    word = 1;
    base = 0;
    rhs = 4'h1;
  endfunction

  initial begin
    failed = 1'b0;
    words[0] = 8'h10;
    words[1] = 8'h20;
    word = 0;
    base = 4;
    word_calls = 0;
    base_calls = 0;
    words[choose_word(0)][choose_base(0) +: 4] += rhs(0);
    `check(words[0], 8'h20);
    `check(words[1], 8'h20);
    `check(word_calls, 1);
    `check(base_calls, 1);

    word = 'bx;
    base = 4;
    words[word][base +: 4] += 1;
    word = 0;
    base = 'bz;
    words[word][base +: 4] ^= 4'hf;
    words[1][base +: 4] += 1;
    `check(words[0], 8'h20);
    `check(words[1], 8'h20);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
