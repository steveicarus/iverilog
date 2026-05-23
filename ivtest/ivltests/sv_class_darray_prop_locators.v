// Regression: class dynamic-array property locator methods.

module test;

  bit failed = 1'b0;

  `define check(val, exp) do     if ((val) !== (exp)) begin       $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, `"val`", exp, val);       failed = 1'b1;     end   while(0)

  class C;
    int d[];
  endclass

  C c;
  int r[$];

  initial begin
    c = new;
    c.d = '{4, 7, 2, 5, 7, 1, 6, 3, 1};

    r = c.d.find() with (item > 3);
    `check(r.size, 5);
    `check(r[0], 4);
    `check(r[4], 6);

    r = c.d.find_first() with (item > 6);
    `check(r.size, 1);
    `check(r[0], 7);

    r = c.d.find_last_index() with (item < 3);
    `check(r.size, 1);
    `check(r[0], 8);

    r = c.d.unique();
    `check(r.size, 7);
    `check(r[0], 4);

    r = c.d.unique_index();
    `check(r.size, 7);
    `check(r[0], 0);

    r = c.d.unique() with (item > 2);
    `check(r.size, 5);
    `check(r[0], 4);
    `check(r[4], 3);

    r = c.d.unique_index() with (item > 2);
    `check(r.size, 6);
    `check(r[0], 0);
    `check(r[5], 7);

    r = c.d.min();
    `check(r.size, 2);
    `check(r[0], 1);
    `check(r[1], 1);

    r = c.d.max() with (item < 7);
    `check(r.size, 1);
    `check(r[0], 6);

    c.d = '{2, 3, 4};
    `check(c.d.product(), 24);
    `check(c.d.sum() with (item + 1), 12);
    `check(c.d.product() with (item + 1), 60);

    c.d = '{1, 2, 3};
    c.d.reverse();
    `check(c.d[0], 3);
    `check(c.d[1], 2);
    `check(c.d[2], 1);

    c.d = '{3, 1, 2};
    c.d.sort();
    `check(c.d[0], 1);
    `check(c.d[1], 2);
    `check(c.d[2], 3);

    if (!failed)
      $display("PASSED");
  end
endmodule
