// Check that it is possible to perform method call on a package scoped
// identifier of a class object.

package P;
  class C;
    function int f1(int x);
      return x + 1;
    endfunction

    function int f2();
      return 10;
    endfunction
  endclass

  C c = new;
endpackage

module test;

  bit failed = 1'b0;

  `define check(expr, val) \
    if (val !== expr) begin \
      $display("FAILED(%0d). `%s` got %b, expected %b.", `__LINE__, `"expr`", expr, val); \
      failed = 1'b1; \
    end

  initial begin
    `check(P::c.f1(10), 11)
    `check(P::c.f2(), 10)

    `check($bits(P::c.f1(10)), $bits(int))
    `check($bits(P::c.f2()), $bits(int))

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
