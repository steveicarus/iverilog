// Check that continue in repeat (1) works without an enclosing loop.

module test;

  function integer f(input integer value);
    f = value;
    repeat (1) begin
      f = f + 1;
      continue;
      f = f + 100;
    end
    f = f + 10;
  endfunction

  localparam integer VALUE = f(0);

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    `check(VALUE, 11);
    `check(f(0), 11);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
