// Check that class copy construction activates a wildcard package import.

package p;
  class C;
    integer value;

    function new(integer value);
      this.value = value;
    endfunction
  endclass

  C source = new(42);
endpackage

module test;

  p::C source = new(1);
  p::C copy_before;
  p::C copy_after;
  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    copy_before = new source;

    begin : imported_scope
      import p::*;
      copy_after = new source;
    end

    `check(copy_before.value, 1);
    `check(copy_after.value, 42);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
