// Regression: queue unique() and unique_index() for integral packed queues.

module top;

  bit failed = 0;

  `define CHK(cond) \
    if (!(cond)) begin \
      $display("FAILED line %0d", `__LINE__); \
      failed = 1; \
    end

  int q[$];
  int u[$];
  int ix[$];

  initial begin
    q.delete();
    q.push_back(1);
    q.push_back(2);
    q.push_back(1);
    q.push_back(3);
    q.push_back(2);

    u = q.unique();
    `CHK(u.size == 3);
    `CHK(u[0] == 1);
    `CHK(u[1] == 2);
    `CHK(u[2] == 3);

    ix = q.unique_index();
    `CHK(ix.size == 3);
    `CHK(ix[0] == 0);
    `CHK(ix[1] == 1);
    `CHK(ix[2] == 3);

    if (!failed)
      $display("PASSED");
  end
endmodule
