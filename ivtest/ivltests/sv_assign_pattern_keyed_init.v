// Check keyed assignment patterns used as variable and parameter
// initializers and as continuous assignment sources.

module test;

  typedef struct packed {
    logic [3:0] a;
    logic [3:0] b;
  } ps_t;

  parameter int P[4] = '{1: 10, default: 0};
  parameter ps_t S = '{b: 4'h2, default: 4'h1};

  int ua[4] = '{3: 30, default: 1};
  ps_t ps = '{a: 4'h4, default: 4'h0};
  logic [3:0] v;
  assign v = '{2: 1'b1, default: 1'b0};

  bit failed = 1'b0;

  `define check(expr, exp) \
    if ((expr) !== (exp)) begin \
      $display("FAILED: `%s`, got %0h, expected %0h", `"expr`", expr, exp); \
      failed = 1'b1; \
    end

  initial begin
    #1;
    `check(P[0], 0)
    `check(P[1], 10)
    `check(P[3], 0)
    `check(S, 8'h12)
    `check(ua[0], 1)
    `check(ua[3], 30)
    `check(ps, 8'h40)
    `check(v, 4'b0100)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
