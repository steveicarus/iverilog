// Check that dynamic arrays and queues of all element types are passed to
// function arguments by value, for automatic and static functions.
module top;
  bit failed = 1'b0;

  task check(input bit ok, input string what);
    if (!ok) begin
      $display("FAILED: %s", what);
      failed = 1'b1;
    end
  endtask

  // dynamic arrays: the callee modifies its copy
  function automatic int da_int(input int a[]);
    a[0] = 99;
    return a.size();
  endfunction

  function automatic int da_bit(input bit [3:0] a[]);
    a[0] = 4'hf;
    return a.size();
  endfunction

  function automatic int da_logic(input logic [7:0] a[]);
    a[0] = 8'hff;
    return a.size();
  endfunction

  function automatic int da_real(input real a[]);
    a[0] = 9.5;
    return a.size();
  endfunction

  function automatic int da_str(input string a[]);
    a[0] = "changed";
    return a.size();
  endfunction

  // queues of real and string
  function automatic int q_real(input real q[$]);
    q[0] = 9.5;
    q.push_back(7.0);
    return q.size();
  endfunction

  function automatic int q_str(input string q[$]);
    q[0] = "changed";
    q.push_back("x");
    return q.size();
  endfunction

  // static functions: the argument is a static variable
  function int sq(input int q[$]);
    int n;
    n = q.size();
    q[0] = q[0] + 100;
    q.push_back(1);
    return n;
  endfunction

  function int sda(input int a[]);
    int n;
    n = a.size();
    a[0] = a[0] + 100;
    return n;
  endfunction

  int ia[];
  bit [3:0] ba[];
  logic [7:0] la[];
  real ra[];
  string sa[];
  real rq[$];
  string sq_[$];
  int q1[$], q2[$];
  int a1[], a2[];

  initial begin
    ia = new[2]; ia[0] = 1; ia[1] = 2;
    check(da_int(ia) == 2, "da_int size");
    check(ia[0] == 1, "da_int caller modified");

    ba = new[2]; ba[0] = 4'h3; ba[1] = 4'h4;
    check(da_bit(ba) == 2, "da_bit size");
    check(ba[0] == 4'h3, "da_bit caller modified");

    la = new[2]; la[0] = 8'h12; la[1] = 8'h34;
    check(da_logic(la) == 2, "da_logic size");
    check(la[0] == 8'h12, "da_logic caller modified");

    ra = new[2]; ra[0] = 1.5; ra[1] = 2.5;
    check(da_real(ra) == 2, "da_real size");
    check(ra[0] == 1.5, "da_real caller modified");

    sa = new[2]; sa[0] = "a"; sa[1] = "b";
    check(da_str(sa) == 2, "da_str size");
    check(sa[0] == "a", "da_str caller modified");

    rq = {1.5, 2.5};
    check(q_real(rq) == 3, "q_real size");
    check(rq.size() == 2 && rq[0] == 1.5, "q_real caller modified");

    sq_ = {"a", "b"};
    check(q_str(sq_) == 3, "q_str size");
    check(sq_.size() == 2 && sq_[0] == "a", "q_str caller modified");

    // static function called twice with different queues
    q1 = {1, 2, 3};
    q2 = {10, 20};
    check(sq(q1) == 3, "static sq call 1");
    check(sq(q2) == 2, "static sq call 2 (stale port from call 1?)");
    check(q1.size() == 3 && q1[0] == 1, "static sq caller q1 modified");
    check(q2.size() == 2 && q2[0] == 10, "static sq caller q2 modified");

    // static function called twice with different dynamic arrays
    a1 = new[3]; a1[0] = 1;
    a2 = new[2]; a2[0] = 10;
    check(sda(a1) == 3, "static sda call 1");
    check(sda(a2) == 2, "static sda call 2");
    check(a1[0] == 1 && a2[0] == 10, "static sda caller modified");

    // the static port keeps the last passed copy between calls
    check(sda.a.size() == 2, "static port size after call");
    check(sda.a[0] == 110, "static port holds last modified copy");

    check(sq.q.size() == 3 && sq.q[0] == 110, "static queue port holds last copy");

    if (!failed) $display("PASSED");
  end
endmodule
