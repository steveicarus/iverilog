// Regression: queue locator methods with `with` — compound predicates and longer sequences.

module top;

  bit failed = 0;

  `define CHK(cond) \
    if (!(cond)) begin \
      $display("FAILED line %0d", `__LINE__); \
      failed = 1; \
    end

  int q[$];
  int res[$];

  initial begin
    q.delete();
    q.push_back(4);
    q.push_back(7);
    q.push_back(2);
    q.push_back(5);
    q.push_back(7);
    q.push_back(1);
    q.push_back(6);
    q.push_back(3);
    q.push_back(1);

    res = q.find() with (item > 3);
    `CHK(res.size == 5);
    `CHK(res[0] == 4);
    `CHK(res[1] == 7);
    `CHK(res[2] == 5);
    `CHK(res[3] == 7);
    `CHK(res[4] == 6);

    res = q.find_index() with (item == 4);
    `CHK(res.size == 1);
    `CHK(res[0] == 0);

    res = q.find_first() with (item < 5 && item >= 3);
    `CHK(res.size == 1);
    `CHK(res[0] == 4);

    res = q.find_first_index() with (item > 5);
    `CHK(res.size == 1);
    `CHK(res[0] == 1);

    res = q.find_last() with (item <= 7 && item > 3);
    `CHK(res.size == 1);
    `CHK(res[0] == 6);

    res = q.find_last_index() with (item < 3);
    `CHK(res.size == 1);
    `CHK(res[0] == 8);

    if (!failed)
      $display("PASSED");
  end
endmodule
