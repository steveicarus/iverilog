// Regression: dynamic array unique() and unique_index().

module top;

  bit failed = 0;

  `define CHK(cond)     if (!(cond)) begin       $display("FAILED line %0d", `__LINE__);       failed = 1;     end

  int a[] = '{1, 2, 1, 3, 2};
  int u[$];
  int ix[$];

  initial begin
    u = a.unique();
    `CHK(u.size == 3);
    `CHK(u[0] == 1);
    `CHK(u[1] == 2);
    `CHK(u[2] == 3);

    ix = a.unique_index();
    `CHK(ix.size == 3);
    `CHK(ix[0] == 0);
    `CHK(ix[1] == 1);
    `CHK(ix[2] == 3);

    if (!failed)
      $display("PASSED");
  end
endmodule
