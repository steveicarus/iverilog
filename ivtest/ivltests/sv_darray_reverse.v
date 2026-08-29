// Regression: dynamic array reverse() ordering method.

module top;

  bit failed = 0;

  `define CHK(cond) \
    if (!(cond)) begin \
      $display("FAILED line %0d", `__LINE__); \
      failed = 1; \
    end

  int a[];

  initial begin
    a = '{1, 2, 3, 4};
    a.reverse();
    `CHK(a[0] === 4 && a[1] === 3 && a[2] === 2 && a[3] === 1);

    a = new [1];
    a[0] = 42;
    a.reverse();
    `CHK(a[0] === 42);

    a = new [0];
    a.reverse();
    `CHK(a.size() === 0);

    if (!failed)
      $display("PASSED");
  end
endmodule
