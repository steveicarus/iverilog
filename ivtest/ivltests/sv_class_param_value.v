// Check value parameters of a parameterized class: defaults, positional and
// named overrides, a typedef of a specialization, and parameters used in
// property widths, array sizes, local parameters and method bodies.

class C #(parameter int N = 4, int M = N * 2);
  localparam int L = N + M;
  logic [N-1:0] vec;
  int arr[N];

  function int width();
    return $bits(vec);
  endfunction

  function int sum();
    return N + M + L;
  endfunction

  function int last();
    arr[N-1] = N;
    return arr[N-1];
  endfunction
endclass

typedef C #(16) c16_t;

module test;

  C a;
  C #() b;
  C #(8) c;
  C #(.M(3)) d;
  C #(.N(2), .M(5)) e;
  c16_t f;
  bit failed = 0;

  task check(input int got, input int exp, input string msg);
    if (got !== exp) begin
      $display("FAILED: %s, got %0d, expected %0d", msg, got, exp);
      failed = 1;
    end
  endtask

  initial begin
    a = new; b = new; c = new; d = new; e = new; f = new;

    check(a.width(), 4, "a.width()");
    check(a.sum(), 24, "a.sum()");
    check(b.width(), 4, "b.width()");
    check(c.width(), 8, "c.width()");
    check(c.sum(), 48, "c.sum()");
    check(d.width(), 4, "d.width()");
    check(d.sum(), 14, "d.sum()");
    check(e.width(), 2, "e.width()");
    check(e.sum(), 14, "e.sum()");
    check(f.width(), 16, "f.width()");
    check(f.last(), 16, "f.last()");
    check(c.last(), 8, "c.last()");

    if (!failed)
      $display("PASSED");
  end

endmodule
