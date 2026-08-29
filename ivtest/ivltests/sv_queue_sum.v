// Regression: queue sum() reduction (integral).

module top;

  bit failed = 0;

  `define CHK(cond) \
    if (!(cond)) begin \
      $display("FAILED line %0d", `__LINE__); \
      failed = 1; \
    end

  int q[$];
  int s;

  initial begin
    q = '{};
    s = q.sum();
    `CHK(s === 0);

    q = '{4, 7, 2};
    s = q.sum();
    `CHK(s === 13);

    if (!failed)
      $display("PASSED");
  end
endmodule
