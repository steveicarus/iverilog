// Check that a class property can shadow a wildcard-imported type name.

package P;

  typedef int T;

endpackage

module test;

  import P::*;

  bit failed = 1'b0;

  `define check(val, exp) do \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, `"val`", exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  class C;
    int T;

    function int get;
      return T;
    endfunction
  endclass

  C obj;

  initial begin
    obj = new;

    obj.T = 47;

    `check(obj.get(), 47);
    `check(obj.T, 47);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
