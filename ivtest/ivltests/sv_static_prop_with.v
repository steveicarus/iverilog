// Check with-clause array methods on class-scope static properties.

module test;

  class C;
    static int q[$];
    static int d[];
  endclass

  C c;
  int r[$];
  int s;
  bit failed = 0;

  `define CHK(cond) \
    if (!(cond)) begin \
      $display("FAILED line %0d", `__LINE__); \
      failed = 1; \
    end

  initial begin
    c = new;
    C::q = '{4, 7, 2, 5, 7};
    C::d = new[5];
    C::d[0] = 4; C::d[1] = 7; C::d[2] = 2; C::d[3] = 5; C::d[4] = 7;

    r = C::q.find(7);
    `CHK(r.size == 2 && r[0] == 7 && r[1] == 7);
    r = C::q.find() with (item > 3);
    `CHK(r.size == 4 && r[0] == 4 && r[3] == 7);
    r = C::q.find with (item > 3);
    `CHK(r.size == 4 && r[0] == 4 && r[3] == 7);
    r = C::q.find_index() with (item == 7);
    `CHK(r.size == 2 && r[0] == 1 && r[1] == 4);
    r = C::q.unique() with (item > 3);
    `CHK(r.size == 3 && r[0] == 4 && r[2] == 5);

    r = C::d.find() with (item > 3);
    `CHK(r.size == 4 && r[0] == 4 && r[3] == 7);
    r = C::d.find_index() with (item == 7);
    `CHK(r.size == 2 && r[0] == 1 && r[1] == 4);

    s = C::q.sum() with (item + 1);
    `CHK(s == 30);
    s = C::q.product() with (item + 1);
    `CHK(s == 5*8*3*6*8);

    // Static properties accessed through an instance already worked.
    r = c.q.find() with (item > 3);
    `CHK(r.size == 4 && r[0] == 4 && r[3] == 7);

    if (!failed)
      $display("PASSED");
  end
endmodule
