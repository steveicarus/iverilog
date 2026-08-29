// Regression: queue reverse() ordering method.

module top;

  bit failed = 0;

  `define CHK(cond) \
    if (!(cond)) begin \
      $display("FAILED line %0d", `__LINE__); \
      failed = 1; \
    end

  int q[$];

  initial begin
    q = '{1, 2, 3, 4};
    q.reverse();
    `CHK(q[0] === 4 && q[1] === 3 && q[2] === 2 && q[3] === 1);

    q = '{99};
    q.reverse();
    `CHK(q[0] === 99);

    q.delete();
    q.reverse();
    `CHK(q.size() === 0);

    if (!failed)
      $display("PASSED");
  end
endmodule
