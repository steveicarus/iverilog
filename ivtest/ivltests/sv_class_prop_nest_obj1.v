// Check that nested object typed class properties can be used as lvalues.

module test;

  bit failed = 1'b0;

  `define check(val, exp) do \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, `"val`", exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  class C;
    C c;
    integer i;
  endclass

  initial begin
    C c1, c2, c3;

    c1 = new;
    c1.c = new;
    c2 = c1.c;

    c3 = new;
    c3.i = 10;
    c1.c.c = c3;
    c3 = c2.c;

    `check(c3.i, 10);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
