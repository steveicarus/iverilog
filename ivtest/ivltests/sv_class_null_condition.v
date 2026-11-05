// Check that class comparisons to null work as conditions.

module test;

  class C;
    C child;
  endclass

  C a, b, objects[2];
  reg [1:0] index;
  reg value;
  integer calls = 0;
  bit failed = 1'b0;

  `define check(cond, exp) \
    if (cond) value = 1'b1; else value = 1'b0; \
    if (value !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"cond`", exp, value); \
      failed = 1'b1; \
    end

  function bit called;
    calls = calls + 1;
    called = 1'b1;
  endfunction

  initial begin
    `check(a == null, 1'b1);
    `check(null != a, 1'b0);
    a = new;
    objects[0] = a;

    `check(a == null, 1'b0);
    `check(null != a, 1'b1);
    `check(a.child == null, 1'b1);
    a.child = a;
    `check(null != a.child, 1'b1);

    `check(objects[0] != null, 1'b1);
    index = 1;
    `check(null == objects[index], 1'b1);
    index = 2;
    `check(objects[index] == null, 1'b1);
    index = 2'bxx;
    `check(objects[index] != null, 1'b0);

    `check((a == null) || (b == null), 1'b1);
    `check((a != null) && (a.child != null), 1'b1);
    `check((a == null) || (b != null), 1'b0);
    `check(1'bx || (a != null), 1'b1);
    `check(1'bx || (b != null), 1'b0);
    `check((a != null) ? (a.child == null) : (b == null), 1'b0);
    `check((b != null) ? (a == null) : (b == null), 1'b1);

    `check((a != null) || called(), 1'b1);
    `check(calls == 0, 1'b1);
    `check((b != null) || called(), 1'b1);
    `check(calls == 1, 1'b1);
    `check((b != null) && called(), 1'b0);
    `check(calls == 1, 1'b1);

    `check(null == null, 1'b1);
    `check(null != null, 1'b0);
    `check((null == null) || called(), 1'b1);
    `check(calls == 1, 1'b1);

    a.child = null;
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
