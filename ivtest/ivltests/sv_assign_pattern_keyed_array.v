// Check keyed assignment patterns (`index: value`, optionally together with
// `default:`) for packed and unpacked arrays.

module test;

  localparam int P = 2;

  typedef struct packed {
    logic [3:0] a;
    logic [3:0] b;
  } ps_t;

  logic [3:0] v;
  logic [3:0][7:0] pa;
  logic [0:3][7:0] pa_up;
  int ua[4];
  int ua_down[3:0];
  int ua_up[1:4];
  int ua_neg[-1:1];
  ps_t sa[3];
  int ua2[2][3];

  bit failed = 1'b0;

  `define check(expr, exp) \
    if ((expr) !== (exp)) begin \
      $display("FAILED: `%s`, got %0h, expected %0h", `"expr`", expr, exp); \
      failed = 1'b1; \
    end

  initial begin
    // Packed vector, index 3 is the left most bit
    v = '{3: 1'b1, default: 1'b0};
    `check(v, 4'b1000)
    v = '{0: 1'b1, 2: 1'b1, default: 1'b0};
    `check(v, 4'b0101)
    v = '{0: 1'b0, 1: 1'b1, 2: 1'b0, 3: 1'b1};
    `check(v, 4'b1010)

    pa = '{1: 8'h11, 3: 8'h33, default: 8'h00};
    `check(pa, 32'h33001100)
    pa_up = '{0: 8'h11, 3: 8'h33, default: 8'h00};
    `check(pa_up, 32'h11000033)

    // Unpacked arrays with all supported range directions
    ua = '{2: 20, default: -1};
    `check(ua[0], -1)
    `check(ua[1], -1)
    `check(ua[2], 20)
    `check(ua[3], -1)

    ua_down = '{3: 3, 0: 1, default: 2};
    `check(ua_down[3], 3)
    `check(ua_down[2], 2)
    `check(ua_down[1], 2)
    `check(ua_down[0], 1)

    ua_up = '{4: 40, default: 0};
    `check(ua_up[1], 0)
    `check(ua_up[4], 40)

    ua_neg = '{-1: 1, 1: 3, default: 2};
    `check(ua_neg[-1], 1)
    `check(ua_neg[0], 2)
    `check(ua_neg[1], 3)

    // No default needed if every index has a key
    ua = '{3: 3, 2: 2, 1: 1, 0: 0};
    `check(ua[0], 0)
    `check(ua[3], 3)

    // Constant expressions as keys
    ua = '{P: 5, P + 1: 6, default: 0};
    `check(ua[1], 0)
    `check(ua[2], 5)
    `check(ua[3], 6)

    // Elements that are structs, with a nested pattern as the value
    sa = '{1: '{a: 4'h1, b: 4'h2}, default: '{default: 4'hf}};
    `check(sa[0], 8'hff)
    `check(sa[1], 8'h12)
    `check(sa[2], 8'hff)

    // Nested arrays
    ua2 = '{1: '{0: 1, default: 2}, default: '{default: 0}};
    `check(ua2[0][0], 0)
    `check(ua2[0][2], 0)
    `check(ua2[1][0], 1)
    `check(ua2[1][1], 2)
    `check(ua2[1][2], 2)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
