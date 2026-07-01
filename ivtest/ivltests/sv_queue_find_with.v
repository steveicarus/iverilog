// Regression: queue find* locator methods with `with (predicate)` (item, index).

module top;

  bit failed = 0;

  `define CHK(cond) \
    if (!(cond)) begin \
      $display("FAILED line %0d", `__LINE__); \
      failed = 1; \
    end

  int q[$];
  int f[$];
  int ix[$];
  int ff[$];
  int fi[$];
  int lf[$];
  int li[$];

  initial begin
    q.delete();
    q.push_back(2);
    q.push_back(1);
    q.push_back(2);
    q.push_back(3);

    f = q.find() with (item == 2);
    `CHK(f.size == 2);
    `CHK(f[0] == 2);
    `CHK(f[1] == 2);

    ix = q.find_index() with (item == 2);
    `CHK(ix.size == 2);
    `CHK(ix[0] == 0);
    `CHK(ix[1] == 2);

    ix = q.find_index() with (index > 0);
    `CHK(ix.size == 3);
    `CHK(ix[0] == 1);
    `CHK(ix[1] == 2);
    `CHK(ix[2] == 3);

    ff = q.find_first() with (item == 3);
    `CHK(ff.size == 1);
    `CHK(ff[0] == 3);
    fi = q.find_first_index() with (item == 2 && index > 0);
    `CHK(fi.size == 1);
    `CHK(fi[0] == 2);

    lf = q.find_last() with (item < 3);
    `CHK(lf.size == 1);
    `CHK(lf[0] == 2);
    li = q.find_last_index() with (item == 2);
    `CHK(li.size == 1);
    `CHK(li[0] == 2);

    ff = q.find_first() with (item == 99);
    `CHK(ff.size == 0);
    fi = q.find_first_index() with (item == 99);
    `CHK(fi.size == 0);

    if (!failed)
      $display("PASSED");
  end
endmodule
