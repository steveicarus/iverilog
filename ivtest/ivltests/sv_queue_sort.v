// Regression: queue sort(), rsort(), shuffle().

module top;

  bit failed = 0;

  `define CHK(cond) \
    if (!(cond)) begin \
      $display("FAILED line %0d", `__LINE__); \
      failed = 1; \
    end

  int q[$];
  int sum0;

  initial begin
    q = '{3, 1, 4, 1, 5};
    q.sort();
    `CHK(q[0] === 1 && q[1] === 1 && q[2] === 3 && q[3] === 4 && q[4] === 5);

    q = '{3, 1, 4};
    q.rsort();
    `CHK(q[0] === 4 && q[1] === 3 && q[2] === 1);

    q = '{10, -2, 7};
    sum0 = q.sum();
    q.shuffle();
    `CHK(q.sum() === sum0);

    if (!failed)
      $display("PASSED");
  end
endmodule
