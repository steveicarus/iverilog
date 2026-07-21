// Regression: iterator index querying item.index in with() (LRM 7.12.4).

module top;

  bit failed = 0;

  `define CHK(cond) \
    if (!(cond)) begin \
      $display("FAILED line %0d", `__LINE__); \
      failed = 1; \
    end

  int arr[] = '{0, 1, 3, 3};
  int q[$];

  initial begin
    q = arr.find with (item == item.index);
    `CHK(q.size == 3);
    `CHK(q[0] == 0);
    `CHK(q[1] == 1);
    `CHK(q[2] == 3);

    if (!failed)
      $display("PASSED");
  end
endmodule
