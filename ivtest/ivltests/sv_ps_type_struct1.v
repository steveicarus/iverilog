// Check that struct types declared in a package can be referenced using a
// scoped type identifier.

bit failed = 1'b0;

`define check(expr, val) \
  if (expr !== val) begin \
    $display("FAILED: %s, expected %0d, got %0d", `"expr`", val, expr); \
    failed = 1'b1; \
  end

package P;

  localparam A = 8;

  typedef struct packed {
    logic [A-1:0] x;
  } T;

endpackage

typedef int T;

module test;
  localparam A = 4;

  typedef int T;

  P::T x;

  initial begin
    x = 8'hff;
    `check(x, 8'hff)
    `check($bits(x), 8)
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
