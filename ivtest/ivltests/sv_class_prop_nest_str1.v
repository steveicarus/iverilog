// Check that nested string typed class properties can be used as lvalues.

module test;

  bit failed = 1'b0;

  `define check(val, exp) do \
    if (val != exp) begin \
      $display("FAILED(%0d). '%s' expected %s, got %s", `__LINE__, `"val`", exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  class C;
    C c;
    string s;
  endclass

  initial begin
    C c1, c2;

    c1 = new;
    c1.c = new;
    c2 = c1.c;

    c1.c.s = "Hello";
    `check(c2.s, "Hello");

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
