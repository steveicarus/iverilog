// Reduction methods on queue and dynamic-array class properties.

module test;

  class C;
    int q[$];
    int d[];
    static int sq[$];
    static int sd[];
  endclass

  C c;
  bit failed = 0;

  `define CHK(val, exp) \
    if ((val) !== (exp)) begin \
      $display("FAILED(%0d). expected %0d, got %0d", `__LINE__, exp, val); \
      failed = 1; \
    end

  initial begin
    c = new;
    c.q = '{1, 3, 5};
    c.d = '{1, 3, 5};
    C::sq = c.q;
    C::sd = c.d;

    `CHK(c.q.and(), 1);
    `CHK(c.q.or(), 7);
    `CHK(c.q.xor(), 7);
    `CHK(c.d.and(), 1);
    `CHK(c.d.or(), 7);
    `CHK(c.d.xor(), 7);

    `CHK(C::sq.and(), 1);
    `CHK(C::sq.or(), 7);
    `CHK(C::sq.xor(), 7);
    `CHK(C::sd.and(), 1);
    `CHK(C::sd.or(), 7);
    `CHK(C::sd.xor(), 7);

    if (!failed)
      $display("PASSED");
  end
endmodule
