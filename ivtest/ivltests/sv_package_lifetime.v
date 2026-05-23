// Check that static lifetime can be specified for variables declared in a
// package.

`define check(val, exp) \
  if (val !== exp) begin \
    $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, `"val`", exp, val); \
    failed = 1'b1; \
  end

package P;
  static int x = 1;
  const static int y = 2;
  var static z;
  var static logic [3:0] w = 4'h3;
endpackage

package automatic Q;
  int a = 4;
  static int b = 5;
endpackage

module test;
  import P::*;
  import Q::*;

  bit failed = 1'b0;

  initial begin
    `check(x, 1)
    `check(y, 2)
    `check(z, 1'bx)
    `check(w, 4'h3)
    `check(a, 4)
    `check(b, 5)

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
