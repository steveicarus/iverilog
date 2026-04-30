// Regression: dynamic array product() reduction with expression.

module top;

  bit failed = 0;

  `define CHK(cond)     if (!(cond)) begin       $display("FAILED line %0d", `__LINE__);       failed = 1;     end

  int a[];
  int p;

  initial begin
    a = '{10, -3, 5};

    p = a.product() with (item > 0);
    `CHK(p === 0);

    p = a.product() with (item + 1);
    `CHK(p === -132);

    if (!failed)
      $display("PASSED");
  end
endmodule
