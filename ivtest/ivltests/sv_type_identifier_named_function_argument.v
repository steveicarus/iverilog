// Check that a named function argument selector can match a visible typedef name.

module test;

  function integer f(input integer T, input integer B);
    f = T + B;
  endfunction

  typedef int T;

  reg failed;
  integer value;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    value = f(.T(40), .B(2));
    `check(value, 42);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
