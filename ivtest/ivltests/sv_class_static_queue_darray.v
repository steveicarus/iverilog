// Check method calls on static queue and dynamic-array class properties
// through an object handle.

class C;
  static int q[$];
  static int d[];
endclass

module test;

  C c;
  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    c = new;

    C::q = '{10, 20};
    `check(c.q.size(), 2);
    c.q.push_back(30);
    `check(C::q.size(), 3);
    `check(c.q[2], 30);

    C::d = new[2];
    C::d[0] = 1;
    C::d[1] = 2;
    `check(c.d.size(), 2);
    c.d.delete();
    `check(C::d.size(), 0);

    if (!failed)
      $display("PASSED");
  end

endmodule
