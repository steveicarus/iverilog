// Check increment and decrement expressions on a real function return value.

module test;

  bit failed = 1'b0;
  real seed, result;

  `define check(val, exp) \
    if (val != exp) begin \
      $display("FAILED(%0d). '%s' expected %g, got %g", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  function real calc(input real value);
    real observed;
    calc = value;
    observed = calc++;
    `check(observed, value);
    `check(calc, value + 1.0);
    observed = ++calc;
    `check(observed, value + 2.0);
    `check(calc, value + 2.0);
    observed = calc--;
    `check(observed, value + 2.0);
    `check(calc, value + 1.0);
    observed = --calc;
    `check(observed, value);
    `check(calc, value);
  endfunction

  initial begin
    seed = 2.5;
    result = calc(seed);
    `check(result, seed);
    seed = -2.5;
    result = calc(seed);
    `check(result, seed);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
