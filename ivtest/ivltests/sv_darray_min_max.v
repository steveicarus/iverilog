// Regression: dynamic array min() and max() locator methods return queues.

module top;

  bit failed = 0;

  `define CHK(cond) \
    if (!(cond)) begin \
      $display("FAILED line %0d", `__LINE__); \
      failed = 1; \
    end

  int a[] = '{4, 7, 2, 5, 7, 1, 6, 3, 1};
  int empty[];
  int r[$];

  initial begin
    r = a.min();
    `CHK(r.size == 2);
    `CHK(r[0] == 1);
    `CHK(r[1] == 1);

    r = a.max();
    `CHK(r.size == 2);
    `CHK(r[0] == 7);
    `CHK(r[1] == 7);

    r = empty.min();
    `CHK(r.size == 0);
    r = empty.max();
    `CHK(r.size == 0);

    if (!failed)
      $display("PASSED");
  end
endmodule
