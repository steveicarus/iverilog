// Regression: dynamic array and()/or()/xor() reductions (LRM 7.12.3).

module top;

  bit failed = 0;

  `define CHK(cond) \
    if (!(cond)) begin \
      $display("FAILED line %0d", `__LINE__); \
      failed = 1; \
    end

  byte b[];
  int y;

  initial begin
    b = '{1, 3, 5, 7};
    y = b.and;
    `CHK(y === 1);

    b = '{1, 2, 3, 4};
    y = b.or;
    `CHK(y === 7);
    y = b.xor;
    `CHK(y === 4);

    b = new[0];
    y = b.and;
    `CHK(y === -1);
    y = b.or;
    `CHK(y === 0);
    y = b.xor;
    `CHK(y === 0);

    if (!failed)
      $display("PASSED");
  end
endmodule
