// Check that unpacked arrays of queues work: push, insert, pop, size and
// indexed reads of the queues in different array words, for vector elements.

module test;

  int q[3][$];
  logic [7:0] b[2][$];
  int m[2][3][$];
  int i;
  int v;
  bit failed = 0;

  task automatic check(bit cond, string msg);
    if (!cond) begin
      $display("FAILED: %s", msg);
      failed = 1;
    end
  endtask

  initial begin
    q[0].push_back(1);
    q[0].push_back(2);
    q[0].push_front(0);
    q[2].push_back(9);
    q[0].insert(1, 5);

    check(q[0].size() == 4, "q[0].size()");
    check(q[1].size() == 0, "q[1].size()");
    check(q[2].size() == 1, "q[2].size()");
    check(q[0][0] == 0 && q[0][1] == 5 && q[0][2] == 1 && q[0][3] == 2,
          "q[0] contents");
    check(q[2][0] == 9, "q[2][0]");

    v = q[0].pop_front();
    check(v == 0 && q[0].size() == 3, "q[0].pop_front()");
    v = q[0].pop_back();
    check(v == 2 && q[0].size() == 2, "q[0].pop_back()");

    i = 1;
    q[i].push_back(42);
    check(q[i][0] == 42 && q[i].size() == 1, "variable word index");

    b[1].push_back(8'hA5);
    check(b[1][0] == 8'hA5 && b[1].size() == 1, "packed element queue");

    m[1][2].push_back(11);
    m[0][1].push_back(22);
    m[1][2].push_back(33);
    check(m[1][2].size() == 2 && m[1][2][0] == 11 && m[1][2][1] == 33,
          "two-dimensional array of queues, m[1][2]");
    check(m[0][1].size() == 1 && m[0][1][0] == 22 && m[0][0].size() == 0,
          "two-dimensional array of queues, m[0][1]");

    if (!failed) $display("PASSED");
  end

endmodule
