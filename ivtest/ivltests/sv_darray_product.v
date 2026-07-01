// Regression: dynamic array product() reduction (integral).

module top;

  bit failed = 0;

  `define CHK(cond)     if (!(cond)) begin       $display("FAILED line %0d", `__LINE__);       failed = 1;     end

  int a[];
  int p;

  initial begin
    a = '{2, 3, 4};
    p = a.product();
    `CHK(p === 24);

    a = '{-2, 3, 5};
    p = a.product();
    `CHK(p === -30);

    if (!failed)
      $display("PASSED");
  end
endmodule
