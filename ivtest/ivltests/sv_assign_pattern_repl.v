// Check replication in assignment patterns (`'{N{value}}` and
// `'{N{a, b}}`), and that it is told apart from a positional item that is a
// replication concatenation.

module test;

  localparam int N = 3;

  typedef struct packed {
    logic [3:0] a;
    logic [3:0] b;
  } ps_t;

  logic [3:0] v;
  logic [3:0][7:0] pa;
  logic [1:0][1:0][3:0] p2;
  int ua[4];
  int ua_up[1:4];
  int ub[N];
  int ua2[2][3];
  ps_t sa[2];
  ps_t ps;
  int q[$];
  int d[];
  int pos[2];
  logic [7:0] w[2];

  bit failed = 1'b0;

  `define check(expr, exp) \
    if ((expr) !== (exp)) begin \
      $display("FAILED: `%s`, got %0h, expected %0h", `"expr`", expr, exp); \
      failed = 1'b1; \
    end

  initial begin
    // Packed vector and arrays
    v = '{4{1'b1}};
    `check(v, 4'b1111)
    v = '{2{1'b1, 1'b0}};
    `check(v, 4'b1010)
    pa = '{4{8'hA5}};
    `check(pa, 32'hA5A5A5A5)
    pa = '{2{8'h12, 8'h34}};
    `check(pa, 32'h12341234)
    p2 = '{2{'{2{4'h7}}}};
    `check(p2, 16'h7777)
    ps = '{2{4'h9}};
    `check(ps, 8'h99)

    // Unpacked arrays
    ua = '{4{5}};
    `check(ua[0], 5)
    `check(ua[3], 5)
    ua = '{2{1, 2}};
    `check(ua[0], 1)
    `check(ua[1], 2)
    `check(ua[2], 1)
    `check(ua[3], 2)
    ua_up = '{2{7, 8}};
    `check(ua_up[1], 7)
    `check(ua_up[4], 8)
    ub = '{N{9}};
    `check(ub[2], 9)

    // Nested patterns and multi-dimensional arrays
    ua2 = '{2{'{3{4}}}};
    `check(ua2[0][2], 4)
    `check(ua2[1][0], 4)
    ua2 = '{'{3{1}}, '{3{2}}};
    `check(ua2[0][1], 1)
    `check(ua2[1][1], 2)
    ua2 = '{2{'{1, 2, 3}}};
    `check(ua2[1][2], 3)
    sa = '{2{'{a: 4'h1, b: 4'h2}}};
    `check(sa[1].a, 4'h1)
    `check(sa[1].b, 4'h2)

    // Replication as a value of a keyed pattern
    ua = '{0: 1, default: 3};
    ua2 = '{0: '{3{6}}, 1: '{3{7}}};
    `check(ua2[0][0], 6)
    `check(ua2[1][2], 7)
    sa = '{0: '{2{4'h1}}, default: '{2{4'h2}}};
    `check(sa[0].b, 4'h1)
    `check(sa[1].a, 4'h2)

    // Dynamic arrays and queues
    d = '{3{4}};
    `check(d.size(), 3)
    `check(d[2], 4)
    q = '{2{1, 2}};
    `check(q.size(), 4)
    `check(q[3], 2)
    q = '{1{7}};
    `check(q.size(), 1)
    `check(q[0], 7)
    d = '{2{5, 6, 7}};
    `check(d.size(), 6)
    `check(d[3], 5)
    `check(d[5], 7)

    // A replication concatenation as a positional item is not a pattern
    w = '{{2{4'h3}}, {2{4'hA}}};
    `check(w[0], 8'h33)
    `check(w[1], 8'hAA)
    pos = '{{2{1'b1}}, 2};
    `check(pos[0], 3)
    `check(pos[1], 2)
    pos = '{2{{2{1'b1}}}};
    `check(pos[0], 3)
    `check(pos[1], 3)

    if (!failed)
      $display("PASSED");
  end

endmodule
