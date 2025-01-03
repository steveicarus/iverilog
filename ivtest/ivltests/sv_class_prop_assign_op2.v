// Check that assignment operators are supported on static class properties.

module test;

  bit failed = 1'b0;

  `define check(val, exp) do \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, `"val`", exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  class C;
    static integer x;
  endclass

  integer i;
  C c;

  initial begin
    c = new;

    c.x = 1;
    c.x += 5;
    `check(c.x, 6);
    c.x -= 2;
    `check(c.x, 4);
    c.x *= 25;
    `check(c.x, 100);
    c.x /= 5;
    `check(c.x, 20);
    c.x %= 3;
    `check(c.x, 2);

    c.x = 'haa;
    c.x &= 'h33;
    `check(c.x, 'h22);
    c.x |= 'h11;
    `check(c.x, 'h33);
    c.x ^= 'h22;
    `check(c.x, 'h11);

    c.x <<= 3;
    `check(c.x, 'h88);
    c.x <<<= 1;
    `check(c.x, 'h110);
    c.x >>= 2;
    `check(c.x, 'h44);
    c.x >>>= 1;
    `check(c.x, 'h22);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
