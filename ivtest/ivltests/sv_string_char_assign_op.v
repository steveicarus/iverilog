// Check assignment operators and index evaluation for string characters.

module test;

  bit failed = 1'b0;
  string value;
  integer index;
  int calls = 0;

  `define check(val, exp) \
    if (val != exp) begin \
      $display("FAILED(%0d). '%s' expected %s, got %s", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  function int select_index();
    calls++;
    return index;
  endfunction

  function byte rhs();
    index = 2;
    return 1;
  endfunction

  initial begin
    value = "abc";
    index = 1;
    value[select_index()] += rhs();
    `check(value, "acc");
    if (calls != 1) begin
      $display("FAILED. Index evaluated %0d times", calls);
      failed = 1'b1;
    end

    {value[0], value[2]} = 16'h7879;
    `check(value, "xcy");
    {value[0], value[2]} += 16'h0101;
    `check(value, "ycz");

    index = 'x;
    value[index] += 1;
    index = -1;
    value[index] += 1;
    index = 3;
    value[index] += 1;
    `check(value, "ycz");

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
