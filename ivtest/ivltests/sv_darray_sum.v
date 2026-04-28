// Regression: dynamic array sum() reduction (integral).

module top;

  bit failed = 0;

  `define CHK(cond) \
    if (!(cond)) begin \
      $display("FAILED line %0d", `__LINE__); \
      failed = 1; \
    end

  int a[];
  int s;

  initial begin
    a = new[0];
    s = a.sum();
    `CHK(s === 0);

    a = '{10, -3, 5};
    s = a.sum();
    `CHK(s === 12);

    if (!failed)
      $display("PASSED");
  end
endmodule
