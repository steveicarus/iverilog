// Check that default values on task methods are supported and it is possible to
// omit any of the arguments.

class C;
  integer r;
  task t(integer x = 1, integer y = 2, integer z = 3);
    r = x + y + z;
  endtask
endclass

module test;

  C c = new;

  bit failed = 1'b0;

  `define check(expr, val) \
    if (expr !== val) begin \
      $display("FAILED. %s, expected %0d, got %0d", `"expr`", val, expr); \
      failed = 1'b1; \
    end

  initial begin
    c.t();
    `check(c.r, 6);
    c.t(4);
    `check(c.r, 9);
    c.t(4, );
    `check(c.r, 9);
    c.t(4, ,);
    `check(c.r, 9);
    c.t(4, 6);
    `check(c.r, 13);
    c.t(4, 6, );
    `check(c.r, 13);
    c.t(4, , 8);
    `check(c.r, 14);
    c.t(4, 6, 8);
    `check(c.r, 18);
    c.t(, 6);
    `check(c.r, 10);
    c.t(, 6, );
    `check(c.r, 10);
    c.t(, 6, 8);
    `check(c.r, 15);
    c.t(, , 8);
    `check(c.r, 11);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
