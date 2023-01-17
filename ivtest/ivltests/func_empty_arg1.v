// Check that it is possible to call functins with empty arguments if they have
// default values.

module test;

  bit failed = 1'b0;

  `define check(expr, val) \
    if (expr !== val) begin \
      $display("FAILED. %s, expected %d, got %d", `"expr`", val, expr); \
      failed = 1'b1; \
    end

  function integer f(integer a = 1, integer b = 2, integer c = 3);
    return a * 100 + b * 10 + c;
  endfunction

  initial begin
    `check(f(4, 5, 6), 456);
    `check(f(4, 5,  ), 453);
    `check(f(4,  , 6), 426);
    `check(f( , 5, 6), 156);
    `check(f(4,  ,  ), 423);
    `check(f( , 5,  ), 153);
    `check(f( ,  , 6), 126);
    `check(f( ,  ,  ), 123);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
