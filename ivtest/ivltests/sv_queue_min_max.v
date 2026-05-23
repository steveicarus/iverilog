// Regression: queue min() and max() locator methods return queues.

module top;

  bit failed = 0;

  `define CHK(cond) \
    if (!(cond)) begin \
      $display("FAILED line %0d", `__LINE__); \
      failed = 1; \
    end

  int q[$];
  int e[$];
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

    r = q.min();
    `CHK(r.size == 2);
    `CHK(r[0] == 1);
    `CHK(r[1] == 1);

    r = q.max();
    `CHK(r.size == 2);
    `CHK(r[0] == 7);
    `CHK(r[1] == 7);

    r = e.min();
    `CHK(r.size == 0);
    r = e.max();
    `CHK(r.size == 0);

    if (!failed)
      $display("PASSED");
  end
endmodule
