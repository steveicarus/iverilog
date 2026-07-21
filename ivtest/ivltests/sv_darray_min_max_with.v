// Regression: dynamic array min/max with(predicate) locator methods.

module top;

  bit failed = 0;

  `define CHK(cond)     if (!(cond)) begin       $display("FAILED line %0d", `__LINE__);       failed = 1;     end

  int a[] = '{4, 7, 2, 5, 7, 1, 6, 3, 1};
  int r[$];

  initial begin
    r = a.min() with (item > 3);
    `CHK(r.size == 1);
    `CHK(r[0] == 4);

    r = a.max() with (item < 7);
    `CHK(r.size == 1);
    `CHK(r[0] == 6);

    r = a.min() with (item > 99);
    `CHK(r.size == 0);

    r = a.max() with (item > 99);
    `CHK(r.size == 0);

    if (!failed)
      $display("PASSED");
  end
endmodule
