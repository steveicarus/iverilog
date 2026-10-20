// Check increment and decrement expressions on an integral function return value.

module test;

  bit failed = 1'b0;
  integer seed, result;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  function integer calc(input integer value);
    integer observed;
    calc = value;
    observed = calc++;
    `check(observed, value);
    `check(calc, value + 1);
    observed = ++calc;
    `check(observed, value + 2);
    `check(calc, value + 2);
    observed = calc--;
    `check(observed, value + 2);
    `check(calc, value + 1);
    observed = --calc;
    `check(observed, value);
    `check(calc, value);
  endfunction

  initial begin
    seed = 2;
    result = calc(seed);
    `check(result, seed);
    seed = -2;
    result = calc(seed);
    `check(result, seed);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
