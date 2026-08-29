// Regression: dynamic array sum() reduction with expression.

module top;

  bit failed = 0;

  `define CHK(cond)     if (!(cond)) begin       $display("FAILED line %0d", `__LINE__);       failed = 1;     end

  int a[];
  int s;

  initial begin
    a = '{10, -3, 5};

    s = a.sum() with (item > 0);
    `CHK(s === 2);

    s = a.sum() with (item + 1);
    `CHK(s === 15);

    if (!failed)
      $display("PASSED");
  end
endmodule
