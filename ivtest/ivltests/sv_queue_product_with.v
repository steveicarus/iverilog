// Regression: queue product() reduction with expression.

module top;

  bit failed = 0;

  `define CHK(cond)     if (!(cond)) begin       $display("FAILED line %0d", `__LINE__);       failed = 1;     end

  int q[$];
  int p;

  initial begin
    q = '{4, 7, 2};

    p = q.product() with (item > 3);
    `CHK(p === 0);

    p = q.product() with (item + 1);
    `CHK(p === 120);

    if (!failed)
      $display("PASSED");
  end
endmodule
