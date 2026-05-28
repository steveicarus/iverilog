// Check that multi-dimensional packed vector class properties are supported.

module test;

  bit failed = 1'b0;

  `define check(val, exp) do begin \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0h, got %0h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end \
  end while (0)

  class C;
    logic [1:0][3:0] x;
    bit [0:1][0:3] y;
  endclass

  C c;

  initial begin
    c = new;

    c.x = 8'h5a;
    c.y = 8'hc3;
    `check(c.x, 8'h5a);
    `check(c.y, 8'hc3);

    c.x += 8'h01;
    c.y ^= 8'hff;
    `check(c.x, 8'h5b);
    `check(c.y, 8'h3c);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
