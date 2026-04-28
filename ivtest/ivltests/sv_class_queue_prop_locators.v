// Regression: class queue property locator methods.

module test;

  bit failed = 1'b0;

  `define check(val, exp) do     if ((val) !== (exp)) begin       $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, `"val`", exp, val);       failed = 1'b1;     end   while (0)

  class C;
    int q[$];
  endclass

  C c;
  int r[$];

  initial begin
    c = new;
    c.q = '{4, 7, 2, 5, 7, 1, 6, 3, 1};

    r = c.q.find() with (item > 3);
    `check(r.size, 5);
    `check(r[0], 4);
    `check(r[4], 6);

    r = c.q.find_last_index() with (item < 3);
    `check(r.size, 1);
    `check(r[0], 8);

    r = c.q.unique() with (item > 2);
    `check(r.size, 5);
    `check(r[0], 4);
    `check(r[4], 3);

    r = c.q.unique_index() with (item > 2);
    `check(r.size, 6);
    `check(r[0], 0);
    `check(r[5], 7);

    r = c.q.min();
    `check(r.size, 2);
    `check(r[0], 1);
    `check(r[1], 1);

    r = c.q.max() with (item < 7);
    `check(r.size, 1);
    `check(r[0], 6);

    r = c.q.max();
    `check(r.size, 2);
    `check(r[0], 7);

    c.q = '{2, 3, 4};
    `check(c.q.product(), 24);
    `check(c.q.sum() with (item + 1), 12);
    `check(c.q.product() with (item + 1), 60);

    if (!failed)
      $display("PASSED");
  end
endmodule
