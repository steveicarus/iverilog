// Check keyed assignment patterns (`member: value`, optionally together with
// `default:`) for packed and unpacked structs.

module test;

  typedef struct packed {
    logic [3:0] a;
    logic [3:0] b;
    logic [7:0] c;
  } ps_t;

  typedef struct packed {
    ps_t p;
    int  i;
  } nested_t;

  typedef struct packed {
    logic [3:0] a;
    logic [3:0] b;
  } ab_t;

  typedef struct packed {
    ab_t p;
    ab_t q;
  } pq_t;

  typedef struct packed {
    ab_t p;
    int  i;
  } pi_t;

  pq_t pq;
  pi_t pi;
  ps_t ps;
  nested_t n;

  bit failed = 1'b0;

  `define check(expr, exp) \
    if ((expr) !== (exp)) begin \
      $display("FAILED: `%s`, got %0h, expected %0h", `"expr`", expr, exp); \
      failed = 1'b1; \
    end

  initial begin
    // Members in declaration order and in arbitrary order
    ps = '{a: 4'h1, b: 4'h2, c: 8'h34};
    `check(ps, 16'h1234)
    ps = '{c: 8'h56, a: 4'h7, b: 4'h8};
    `check(ps, 16'h7856)

    // Members that are not listed get the default
    ps = '{b: 4'h9, default: 0};
    `check(ps, 16'h0900)
    ps = '{c: 8'h5a, default: 4'hf};
    `check(ps, 16'hff5a)

    // Nested patterns
    n = '{i: 5, p: '{a: 4'h1, default: 4'h0}};
    `check(n.p, 16'h1000)
    `check(n.i, 5)
    n = '{p: '{c: 8'hff, default: 4'h0}, default: 3};
    `check(n.p, 16'h00ff)
    `check(n.i, 3)

    // A default is applied to the elements of struct members
    pq = '{default: 4'h3};
    `check(pq, 16'h3333)
    pq = '{q: '{default: 4'h1}, default: 4'h2};
    `check(pq, 16'h2211)
    pi = '{default: 1};
    `check(pi.p, 8'h11)
    `check(pi.i, 1)

    // The default may come first
    ps = '{default: 1, a: 2};
    `check(ps, 16'h2101)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
