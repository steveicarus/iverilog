// Regression: queue product() reduction (integral).

module top;

  bit failed = 0;

  `define CHK(cond)     if (!(cond)) begin       $display("FAILED line %0d", `__LINE__);       failed = 1;     end

  int q[$];
  int p;

  initial begin
    q = '{2, 3, 4};
    p = q.product();
    `CHK(p === 24);

    q = '{-2, 3, 5};
    p = q.product();
    `CHK(p === -30);

    if (!failed)
      $display("PASSED");
  end
endmodule
