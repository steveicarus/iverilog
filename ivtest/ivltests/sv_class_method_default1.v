// Check that default values on function methods are supported and it is
// possible to omit any of the arguments.

class C;
  function integer f(integer x = 1, integer y = 2, integer z = 3);
    return x + y + z;
  endfunction
endclass

module test;

  C c = new;

  bit failed = 1'b0;

  `define check(expr, val) \
    if (expr !== val) begin \
      $display("FAILED. %s, expected %d, got %d", `"expr`", val, expr); \
      failed = 1'b1; \
    end

  initial begin
    `check(c.f(), 6);
    `check(c.f(4), 9);
    `check(c.f(4, ), 9);
    `check(c.f(4, , ), 9);
    `check(c.f(4, 6), 13);
    `check(c.f(4, 6, ), 13);
    `check(c.f(4, , 8), 14);
    `check(c.f(4, 6, 8), 18);
    `check(c.f(, 6), 10);
    `check(c.f(, 6, ), 10);
    `check(c.f(, 6 ,8), 15);
    `check(c.f(, , 8), 11);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
