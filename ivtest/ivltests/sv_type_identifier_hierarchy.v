// Check that a type identifier can be shadowed by a forward-referenced
// hierarchy identifier.

package p;
  typedef integer SCOPE;
endpackage

import p::*;

module test;

  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;

    SCOPE.value = 32'd17;
    `check(SCOPE.value, 32'd17)

    if (!failed) begin
      $display("PASSED");
    end
  end

  generate
    if (1) begin : SCOPE
      integer value;
    end
  endgenerate

endmodule
