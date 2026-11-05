// Check that braces without an apostrophe inside an assignment pattern are
// handled as nested patterns when the element is an unpacked array.

module test;

  typedef struct packed {
    logic [7:0] a;
    logic [7:0] b;
  } ps_t;

  int ua2[2][2];
  int ua3[2][2][2];
  int ud[2][3];
  ps_t pa[2];
  logic [7:0] w[2];

  bit failed = 1'b0;

  `define check(expr, exp) \
    if ((expr) !== (exp)) begin \
      $display("FAILED: `%s`, got %0h, expected %0h", `"expr`", expr, exp); \
      failed = 1'b1; \
    end

  initial begin
    ua2 = '{{0, 1}, {2, 3}};
    `check(ua2[0][0], 0)
    `check(ua2[0][1], 1)
    `check(ua2[1][0], 2)
    `check(ua2[1][1], 3)

    ua3 = '{{{1, 2}, {3, 4}}, '{'{5, 6}, {7, 8}}};
    `check(ua3[0][0][1], 2)
    `check(ua3[0][1][0], 3)
    `check(ua3[1][0][0], 5)
    `check(ua3[1][1][1], 8)

    // Replication inside of the nested braces
    ud = '{{3{1}}, {3{2}}};
    `check(ud[0][2], 1)
    `check(ud[1][0], 2)
    ud = '{2{{1, 2, 3}}};
    `check(ud[1][1], 2)

    // A packed element still takes a concatenation as a vector
    pa = '{{8'h12, 8'h34}, {8'h56, 8'h78}};
    `check(pa[0], 16'h1234)
    `check(pa[1], 16'h5678)
    w = '{{4'h1, 4'h2}, {4'h3, 4'h4}};
    `check(w[0], 8'h12)
    `check(w[1], 8'h34)

    if (!failed)
      $display("PASSED");
  end

endmodule
