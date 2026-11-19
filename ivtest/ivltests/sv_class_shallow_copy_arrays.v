// Check that a shallow copy of a class object (new obj) copies queue and
// dynamic array properties, while a class handle property still refers to
// the same object.

module test;

  class Inner;
    int n;
  endclass

  class C;
    int q[$];
    int d[];
    real rq[$];
    real rd[];
    string sq[$];
    string sd[];
    Inner h;
  endclass

  bit failed = 1'b0;
  task check(bit ok, string what);
    if (!ok) begin
      $display("FAILED: %s", what);
      failed = 1'b1;
    end
  endtask

  C c, c2, n1, n2;
  Inner h;
  int r[$];
  real rr[$];
  string ss[$];

  initial begin
    c = new;
    c.q = '{1, 2};
    c.d = '{1, 2, 3};
    rr = '{1.5, 2.5};
    c.rq = rr;
    c.rd = '{1.5, 2.5};
    ss = '{"a", "b"};
    c.sq = ss;
    c.sd = '{"a", "b"};
    c.h = new;

    c2 = new c;
    c2.q[0] = 9;
    c2.q[$+1] = 4;
    check(c.q.size() == 2 && c.q[0] == 1, "queue, change copy");
    c.q[1] = 8;
    check(c2.q[1] == 2, "queue, change original");

    c2.d[0] = 9;
    check(c.d[0] == 1, "darray, change copy");
    c.d[1] = 8;
    check(c2.d[1] == 2, "darray, change original");

    c2.rq[0] = 9.5;
    c2.rq[$+1] = 4.5;
    check(c.rq.size() == 2 && c.rq[0] == 1.5, "real queue, change copy");
    c.rq[1] = 8.5;
    check(c2.rq[1] == 2.5, "real queue, change original");

    c2.rd[0] = 9.5;
    check(c.rd[0] == 1.5, "real darray, change copy");
    c.rd[1] = 8.5;
    check(c2.rd[1] == 2.5, "real darray, change original");

    c2.sq[0] = "z";
    c2.sq[$+1] = "d";
    check(c.sq.size() == 2 && c.sq[0] == "a", "string queue, change copy");
    c.sq[1] = "y";
    check(c2.sq[1] == "b", "string queue, change original");

    c2.sd[0] = "z";
    check(c.sd[0] == "a", "string darray, change copy");
    c.sd[1] = "y";
    check(c2.sd[1] == "b", "string darray, change original");

    c2.d = new[5];
    check(c.d.size() == 3, "darray, new[] on copy");

    // Properties that were never written.
    n1 = new;
    n2 = new n1;
    n2.q[$+1] = 1;
    n2.rq[$+1] = 1.5;
    n2.sq[$+1] = "x";
    n2.d = new[2];
    n2.rd = new[2];
    n2.sd = new[2];
    check(n1.q.size() == 0, "unwritten queue");
    check(n1.rq.size() == 0, "unwritten real queue");
    check(n1.sq.size() == 0, "unwritten string queue");
    check(n1.d.size() == 0, "unwritten darray");
    check(n1.rd.size() == 0, "unwritten real darray");
    check(n1.sd.size() == 0, "unwritten string darray");
    check(n2.q.size() == 1 && n2.d.size() == 2, "unwritten, copy keeps its own");

    c.h.n = 5;
    c2 = new c;
    h = c2.h;
    h.n = 7;
    h = c.h;
    check(h.n == 7, "class handle stays shared");

    if (failed)
      $display("FAILED");
    else
      $display("PASSED");
  end

endmodule
