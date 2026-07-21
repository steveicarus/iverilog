// Regression: dynamic array locator methods (find* with predicate); results are queues.

module top;

  bit failed = 0;

  `define CHK(cond) \
    if (!(cond)) begin \
      $display("FAILED line %0d", `__LINE__); \
      failed = 1; \
    end

  int array[] = '{4, 7, 2, 5, 7, 1, 6, 3, 1};
  int res[$];
  string s[] = '{"hello", "sad", "hello", "world"};
  string qs[$];
  int qi[$];

  initial begin
    res = array.find() with (item > 3);
    `CHK(res.size == 5);
    `CHK(res[0] == 4);
    `CHK(res[1] == 7);
    `CHK(res[2] == 5);
    `CHK(res[3] == 7);
    `CHK(res[4] == 6);

    res = array.find_index() with (item == 4);
    `CHK(res.size == 1);
    `CHK(res[0] == 0);

    res = array.find_first() with (item < 5 && item >= 3);
    `CHK(res.size == 1);
    `CHK(res[0] == 4);

    res = array.find_first_index() with (item > 5);
    `CHK(res.size == 1);
    `CHK(res[0] == 1);

    res = array.find_last() with (item <= 7 && item > 3);
    `CHK(res.size == 1);
    `CHK(res[0] == 6);

    res = array.find_last_index() with (item < 3);
    `CHK(res.size == 1);
    `CHK(res[0] == 8);

    qs = s.find with (item == "sad");
    `CHK(qs.size == 1);
    `CHK(qs[0] == "sad");

    qs = s.find_first with (item == "hello");
    `CHK(qs.size == 1);
    `CHK(qs[0] == "hello");

    qs = s.find_last with (item == "hello");
    `CHK(qs.size == 1);
    `CHK(qs[0] == "hello");

    qi = s.find_index with (item == "world");
    `CHK(qi.size == 1);
    `CHK(qi[0] == 3);

    qi = s.find_first_index with (item == "hello");
    `CHK(qi.size == 1);
    `CHK(qi[0] == 0);

    qi = s.find_last_index with (item == "hello");
    `CHK(qi.size == 1);
    `CHK(qi[0] == 2);

    if (!failed)
      $display("PASSED");
  end
endmodule
