// Regression: queue min/max with(predicate) locator methods.

module top;

  bit failed = 0;

  `define CHK(cond)     if (!(cond)) begin       $display("FAILED line %0d", `__LINE__);       failed = 1;     end

  int q[$];
  int r[$];

  initial begin
    q.delete();
    q.push_back(4);
    q.push_back(7);
    q.push_back(2);
    q.push_back(5);
    q.push_back(7);
    q.push_back(1);
    q.push_back(6);
    q.push_back(3);
    q.push_back(1);

    r = q.min() with (item > 3);
    `CHK(r.size == 1);
    `CHK(r[0] == 4);

    r = q.max() with (item < 7);
    `CHK(r.size == 1);
    `CHK(r[0] == 6);

    r = q.min() with (item > 99);
    `CHK(r.size == 0);

    r = q.max() with (item > 99);
    `CHK(r.size == 0);

    if (!failed)
      $display("PASSED");
  end
endmodule
