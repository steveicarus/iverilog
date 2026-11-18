// Regression: $ as an expression in the index of a queue class property,
// e.g. [$-1] and the append form [$+1] = v, for reads and writes, through
// a handle, through this, in a parameterized class and in a derived class.

module test;

  class C;
    string s[$];
    real r[$];
    int i[$];
    int idx[$];

    function void setup();
      s.push_back("a"); s.push_back("b"); s.push_back("c");
      r.push_back(1.5); r.push_back(2.5); r.push_back(3.5);
      i.push_back(7); i.push_back(8); i.push_back(9);
      idx.push_back(1);
    endfunction

    function void append();
      s[$+1] = "d"; r[$+1] = 4.5; i[$+1] = 10;
    endfunction

    function void append_this();
      this.s[$+1] = "e"; this.r[$+1] = 5.5; this.i[$+1] = 11;
    endfunction

    function string prev();
      return this.s[$-1];
    endfunction
  endclass

  class P #(type T = int);
    T q[$];

    function void add(T v);
      q[$+1] = v;
    endfunction

    function T prev();
      return q[$-1];
    endfunction
  endclass

  class B;
    string q[$];
  endclass

  class D extends B;
    function void add(string v);
      q[$+1] = v;
    endfunction
  endclass

  bit failed = 1'b0;

  `define check(val, exp) do \
    if (val !== exp) begin \
      $display("FAILED(%0d). expected %0d, got %0d", `__LINE__, exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  `define checks(val, exp) do \
    if (val != exp) begin \
      $display("FAILED(%0d). expected '%s', got '%s'", `__LINE__, exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  `define checkr(val, exp) do \
    if (val != exp) begin \
      $display("FAILED(%0d). expected %f, got %f", `__LINE__, exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  C c;
  P#(string) ps;
  P#(real) pr;
  P#(int) pi;
  D d;

  initial begin
    c = new;
    c.setup();

    // Reads relative to the last element
    `checks(c.s[$-1], "b");
    `checkr(c.r[$-1], 2.5);
    `check(c.i[$-2], 7);
    `checks(c.s[$-5], "");
    `checks(c.s[c.idx[$]], "b");
    `checks(c.s[$-c.idx[$]], "b");
    `checks(c.prev(), "b");

    // Append with [$+1]
    c.append();
    `check(c.s.size(), 4);
    `checks(c.s[3], "d");
    `checkr(c.r[3], 4.5);
    `check(c.i[3], 10);
    c.append_this();
    `check(c.s.size(), 5);
    `checks(c.s[$], "e");
    `checkr(c.r[$], 5.5);
    `check(c.i[$], 11);
    c.s[$+1] = "f";
    c.r[$+1] = 6.5;
    c.i[$+1] = 12;
    `checks(c.s[$], "f");
    `checkr(c.r[$], 6.5);
    `check(c.i[$], 12);
    `checks(c.s[$-1], "e");

    // Writes relative to the last element
    c.s[$-1] = "x";
    c.r[$-1] = 0.5;
    c.i[$-1] = 0;
    `checks(c.s[4], "x");
    `checkr(c.r[4], 0.5);
    `check(c.i[4], 0);
    `check(c.s.size(), 6);

    // Parameterized class
    ps = new; pr = new; pi = new;
    ps.add("p"); ps.add("q");
    pr.add(1.25); pr.add(2.25);
    pi.add(5); pi.add(6);
    `checks(ps.prev(), "p");
    `checkr(pr.prev(), 1.25);
    `check(pi.prev(), 5);
    `checks(ps.q[$], "q");
    ps.q[$+1] = "r";
    pr.q[$+1] = 3.25;
    pi.q[$+1] = 7;
    `check(ps.q.size(), 3);
    `checks(ps.q[$-2], "p");
    `checkr(pr.q[$], 3.25);
    `check(pi.q[$], 7);

    // Inherited property
    d = new;
    d.add("u"); d.add("v");
    `checks(d.q[$], "v");
    `checks(d.q[$-1], "u");
    d.q[$+1] = "w";
    `check(d.q.size(), 3);
    `checks(d.q[$], "w");

    // Empty queue: [$+1] appends the first element
    c = new;
    c.s[$+1] = "first";
    c.r[$+1] = 1.0;
    `check(c.s.size(), 1);
    `checks(c.s[0], "first");
    `checkr(c.r[0], 1.0);

    if (!failed)
      $display("PASSED");
  end

endmodule
