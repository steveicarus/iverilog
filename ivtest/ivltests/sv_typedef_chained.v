// Check that chained type definitions declared in different scopes work
// correctly.

package P1;
  localparam A = 8;
  typedef logic [A-1:0] T;
endpackage

package P2;
  localparam A = 4;
  typedef P1::T T;
endpackage

bit failed = 1'b0;

`define check(expr, val) \
  if (expr !== val) begin \
    $display("FAILED: %s, expected %0d, got %0d", `"expr`", val, expr); \
    failed = 1'b1; \
  end

module test;
  localparam A = 2;

  typedef P2::T T;

  T x;
  P1::T y;
  P2::T z;

  initial begin
    x = 8'hff;
    y = 8'hff;
    z = 8'hff;

    `check(x, 8'hff);
    `check($bits(T), 8);
    `check($bits(x), 8);
    `check(y, 8'hff);
    `check($bits(y), 8);
    `check($bits(P1::T), 8);
    `check(z, 8'hff);
    `check($bits(z), 8);
    `check($bits(P2::T), 8);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
