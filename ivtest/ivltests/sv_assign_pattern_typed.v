// Check typed assignment patterns, `type'{...}`. The pattern takes the given
// type, so it also works where there is no assignment target to supply one.

package pk;
  typedef struct packed {
    logic [3:0] a;
    logic [3:0] b;
  } p_t;
endpackage

module sub(input pk::p_t i, output [7:0] o);
  assign o = i;
endmodule

module gen #(parameter type T = logic [15:0]);
  T x;
  initial x = T'{a: 8'h12, b: 8'h34};
  wire [15:0] w = x;
endmodule

module test;

  typedef struct packed {
    logic [7:0] a;
    logic [7:0] b;
  } s_t;

  typedef struct packed {
    logic [7:0] c;
    logic [7:0] d;
  } r_t;

  typedef enum logic [1:0] {A, B, C} e_t;

  typedef struct packed {
    e_t e;
    logic [1:0] z;
  } es_t;

  typedef struct packed {
    s_t x;
    logic [3:0] y;
  } o_t;

  typedef int arr_t[4];
  typedef int arr2_t[4];
  typedef real ra_t[2];
  typedef logic [1:0][3:0] pa_t;
  typedef int q_t[$];
  typedef int da_t[];

  s_t s;
  r_t r;
  o_t o;
  es_t es;
  pa_t pa;
  arr_t ar;
  ra_t ra;
  q_t q;
  da_t da;
  logic [15:0] v;
  int a4[4];

  wire [7:0] pw;
  sub si(.i(pk::p_t'{a: 4'h1, b: 4'h2}), .o(pw));
  wire [15:0] cw = s_t'{a: 8'h56, b: 8'h78};
  s_t init = s_t'{a: 3, b: 4};
  gen #(s_t) gi();

  function s_t swap(input s_t x);
    return s_t'{a: x.b, b: x.a};
  endfunction

  function s_t fives();
    return s_t'{default: 5};
  endfunction

  bit failed = 1'b0;

  `define check(expr, exp) \
    if ((expr) !== (exp)) begin \
      $display("FAILED: `%s`, got %0h, expected %0h", `"expr`", expr, exp); \
      failed = 1'b1; \
    end

  initial begin
    #1;

    // Keyed and positional patterns
    s = s_t'{a: 8'h12, b: 8'h34};
    `check(s, 16'h1234)
    s = s_t'{8'h56, 8'h78};
    `check(s, 16'h5678)
    s = s_t'{b: 1, default: 2};
    `check(s, 16'h0201)

    // As an operand of an expression
    `check(s_t'{a: 1, b: 2} == 16'h0102, 1'b1)
    `check(s_t'{a: 1, b: 2} + 1, 32'h0103)
    `check({s_t'{a: 1, b: 2}, 4'hf}, 20'h0102f)
    case (s_t'{a: 1, b: 2})
      16'h0102: ;
      default: begin
	$display("FAILED: case");
	failed = 1'b1;
      end
    endcase

    // As function argument and return value, initialiser, port connection,
    // continuous assignment and type parameter
    s = swap(s_t'{a: 1, b: 2});
    `check(s, 16'h0201)
    s = fives();
    `check(s, 16'h0505)
    `check(init, 16'h0304)
    `check(pw, 8'h12)
    `check(cw, 16'h5678)
    `check(gi.w, 16'h1234)

    // Assigned to a different but compatible type
    v = s_t'{a: 8'h12, b: 8'h34};
    `check(v, 16'h1234)
    r = s_t'{a: 8'h56, b: 8'h78};
    `check(r, 16'h5678)

    // Nested typed patterns and enum members
    o = o_t'{x: s_t'{a: 1, b: 2}, y: 3};
    `check(o, 20'h01023)
    es = es_t'{e: B, z: 1};
    `check(es, 4'h5)

    // Packed arrays
    pa = pa_t'{4'h1, 4'h2};
    `check(pa, 8'h12)
    pa = pa_t'{default: 4'h3};
    `check(pa, 8'h33)

    // Unpacked arrays, queues and dynamic arrays
    ar = arr_t'{1, 2, 3, 4};
    `check(ar[0], 1)
    `check(ar[3], 4)
    ar = arr_t'{default: 7};
    `check(ar[2], 7)
    ar = arr_t'{1: 5, default: 0};
    `check(ar[1], 5)
    `check(ar[0], 0)
    a4 = arr2_t'{9, 8, 7, 6};
    `check(a4[1], 8)
    ra = ra_t'{1.5, 2.5};
    if (ra[0] != 1.5 || ra[1] != 2.5) begin
      $display("FAILED: real array");
      failed = 1'b1;
    end
    q = q_t'{1, 2, 3};
    `check(q.size(), 3)
    `check(q[2], 3)
    da = da_t'{4, 5};
    `check(da.size(), 2)
    `check(da[1], 5)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
