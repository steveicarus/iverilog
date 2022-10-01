// Check that it is possible to perform method call on a package scoped
// identifier of an enum.

package P;
  enum integer {
    A, B
  } e = A;
endpackage

module test;

  bit failed = 1'b0;

  `define check(expr, val) \
    if (val !== expr) begin \
      $display("FAILED(%0d). `%s` got %b, expected %b.", `__LINE__, `"expr`", expr, val); \
      failed = 1'b1; \
    end

  initial begin
    // Calling without a parameter. Both variants are allowed
    `check(P::e.first, P::A)
    `check(P::e.first(), P::A)

    // Is the width reported correctly for both variants?
    `check($bits(P::e.first), $bits(integer))
    `check($bits(P::e.first()), $bits(integer))

    // Calling with a parameter
    `check(P::e.next(1), P::B)

    `check($bits(P::e.next(1)), $bits(integer))

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
