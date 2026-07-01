// Regression: queue sum() reduction with expression.

module top;

  bit failed = 0;

  `define CHK(cond)     if (!(cond)) begin       $display("FAILED line %0d", `__LINE__);       failed = 1;     end

  int q[$];
  int s;

  initial begin
    q = '{4, 7, 2, 5};

    s = q.sum() with (item > 3);
    `CHK(s === 3);

    s = q.sum() with (item * 2);
    `CHK(s === 36);

    if (!failed)
      $display("PASSED");
  end
endmodule
