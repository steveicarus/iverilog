// Copyright (c) 2026 Srinivasan Venkataramanan (AsFigo, https://www.asfigo.com)
// br_gh1812.v -- Regression for GitHub issue #1812
// delete() and delete(index) on a queue class property, called from inside
// the class and from outside it.

class C;
  int q[$];
  function void fill();
    q.push_back(10);
    q.push_back(20);
    q.push_back(30);
  endfunction
  function void clear_this();
    this.q.delete();
  endfunction
  function void clear_bare();
    q.delete();
  endfunction
  function void del_one(int i);
    q.delete(i);
  endfunction
endclass

module top;
  C c = new;
  int errors = 0;

  task check(string what, int got, int expected);
    if (got != expected) begin
      $display("FAILED: %s: got %0d, expected %0d", what, got, expected);
      errors += 1;
    end
  endtask

  initial begin
    c.fill();
    check("size after fill", c.q.size(), 3);

    c.del_one(1);
    check("size after delete(1)", c.q.size(), 2);
    check("q[0] after delete(1)", c.q[0], 10);
    check("q[1] after delete(1)", c.q[1], 30);

    c.clear_bare();
    check("size after delete()", c.q.size(), 0);

    c.fill();
    c.clear_this();
    check("size after this.delete()", c.q.size(), 0);

    c.fill();
    c.q.delete(0);
    check("size after c.q.delete(0)", c.q.size(), 2);
    check("q[0] after c.q.delete(0)", c.q[0], 20);

    c.q.delete();
    check("size after c.q.delete()", c.q.size(), 0);

    c.fill();
    check("size after refill", c.q.size(), 3);

    if (errors == 0)
      $display("PASSED");
  end
endmodule
