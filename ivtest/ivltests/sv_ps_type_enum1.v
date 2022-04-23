// Check that enum types declared in a package can be referenced using a scoped
// type identifier.

bit failed = 1'b0;

`define check(expr, val) \
  if (expr !== val) begin \
    $display("FAILED: %s, expected %0d, got %0d", `"expr`", val, expr); \
    failed = 1'b1; \
  end

package P;

  localparam X = 8;

  typedef enum logic [X-1:0] {
    A, B = X
  } T;

endpackage

module test;
  localparam X = 4;

  typedef int T;

  P::T x = P::B;

  initial begin
    `check(x, P::B)
    `check(x, 8)
    `check($bits(x), 8);
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
