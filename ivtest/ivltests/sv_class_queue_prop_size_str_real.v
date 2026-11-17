// Regression: size() of string and real queue-typed class properties
// must report the number of elements. Previously only vector queues
// were counted and real/string queues always reported 0.

module test;

  class C;
    string s[$];
    real r[$];
    int i[$];

    function void add(string x, real y);
      s.push_back(x);
      r.push_back(y);
    endfunction

    function void add_front(string x, real y);
      s.push_front(x);
      r.push_front(y);
    endfunction

    function void ins(string x, real y);
      s.insert(0, x);
      r.insert(0, y);
    endfunction
  endclass

  bit failed = 1'b0;

  `define check(val, exp) do \
    if (val !== exp) begin \
      $display("FAILED(%0d). expected %0d, got %0d", `__LINE__, exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  C c;
  string str;
  real rv;

  initial begin
    c = new;
    `check(c.s.size(), 0);
    `check(c.r.size(), 0);

    c.add("a", 1.5);
    `check(c.s.size(), 1);
    `check(c.r.size(), 1);

    c.add_front("b", 2.5);
    `check(c.s.size(), 2);
    `check(c.r.size(), 2);

    c.ins("c", 3.5);
    `check(c.s.size(), 3);
    `check(c.r.size(), 3);
    `check($size(c.s), 3);

    c.s.push_back("d");
    c.r.push_back(4.5);
    `check(c.s.size(), 4);
    `check(c.r.size(), 4);
    `check(c.i.size(), 0);

    str = c.s.pop_front();
    rv = c.r.pop_back();
    `check(c.s.size(), 3);
    `check(c.r.size(), 3);
    if (str != "c") begin
      $display("FAILED. pop_front string '%s'", str);
      failed = 1'b1;
    end
    if (rv != 4.5) begin
      $display("FAILED. pop_back real %f", rv);
      failed = 1'b1;
    end

    if (!failed)
      $display("PASSED");
  end

endmodule
