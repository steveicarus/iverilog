// Check that break in repeat (1) does not affect an enclosing loop.

module test;

  function integer f(input integer count);
    f = 0;
    repeat (count) begin
      repeat (1) begin
        f = f + 1;
        break;
        f = f + 100;
      end
      f = f + 10;
    end
  endfunction

  localparam integer VALUE = f(3);

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    `check(VALUE, 33);
    `check(f(3), 33);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
