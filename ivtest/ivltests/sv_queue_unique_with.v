// Regression: queue unique()/unique_index() with predicate.

module top;

  bit failed = 0;

  `define CHK(cond)     if (!(cond)) begin       $display("FAILED line %0d", `__LINE__);       failed = 1;     end

  int q[$];
  int r[$];

  initial begin
    q = '{4, 7, 2, 5, 7, 1, 6, 3, 1};

    r = q.unique() with (item > 2);
    `CHK(r.size == 5);
    `CHK(r[0] == 4);
    `CHK(r[1] == 7);
    `CHK(r[2] == 5);
    `CHK(r[3] == 6);
    `CHK(r[4] == 3);

    r = q.unique_index() with (item > 2);
    `CHK(r.size == 6);
    `CHK(r[0] == 0);
    `CHK(r[1] == 1);
    `CHK(r[2] == 3);
    `CHK(r[3] == 4);
    `CHK(r[4] == 6);
    `CHK(r[5] == 7);

    if (!failed)
      $display("PASSED");
  end
endmodule
