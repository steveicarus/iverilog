// Regression: dynamic array sort(), rsort(), shuffle().

module top;

  bit failed = 0;

  `define CHK(cond) \
    if (!(cond)) begin \
      $display("FAILED line %0d", `__LINE__); \
      failed = 1; \
    end

  int a[];
  int sum0;

  initial begin
    a = '{3, 1, 4, 1, 5};
    a.sort();
    `CHK(a[0] === 1 && a[1] === 1 && a[2] === 3 && a[3] === 4 && a[4] === 5);

    a = '{3, 1, 4};
    a.rsort();
    `CHK(a[0] === 4 && a[1] === 3 && a[2] === 1);

    a = '{10, -2, 7};
    sum0 = a.sum();
    a.shuffle();
    `CHK(a.sum() === sum0);

    if (!failed)
      $display("PASSED");
  end
endmodule
