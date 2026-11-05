// Check that `default:` in an assignment pattern applies to every element of
// packed vectors, packed arrays, unpacked arrays and structs.

module test;

  typedef struct packed {
    logic [3:0] a;
    logic [3:0] b;
  } ps_t;

  typedef struct packed {
    ps_t p;
    int  i;
  } ns_t;

  logic [3:0] v;
  logic [3:0][7:0] pa;
  int ua[4];
  int ua_down[3:0];
  int ua_up[0:3];
  int ua2[2][3];
  ps_t ps;
  ns_t ns;

  bit failed = 1'b0;

  `define check(expr, exp) \
    if ((expr) !== (exp)) begin \
      $display("FAILED: `%s`, got %0h, expected %0h", `"expr`", expr, exp); \
      failed = 1'b1; \
    end

  initial begin
    v = '{default: 1'b1};
    `check(v, 4'b1111)
    v = '{default: 1'bx};
    `check(v, 4'bxxxx)

    pa = '{default: 8'ha5};
    `check(pa, 32'ha5a5a5a5)
    pa = '{default: '1};
    `check(pa, 32'hffffffff)

    ua = '{default: 7};
    `check(ua[0], 7)
    `check(ua[1], 7)
    `check(ua[2], 7)
    `check(ua[3], 7)

    ua_down = '{default: -1};
    `check(ua_down[0], -1)
    `check(ua_down[3], -1)

    ua_up = '{default: 5};
    `check(ua_up[0], 5)
    `check(ua_up[3], 5)

    ua2 = '{default: '{default: 9}};
    `check(ua2[0][0], 9)
    `check(ua2[1][2], 9)

    ps = '{default: 4'h3};
    `check(ps, 8'h33)

    ns = '{default: '1};
    `check(ns, 40'hffffffffff)
    ns = '{default: 0};
    `check(ns, 40'h0)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
