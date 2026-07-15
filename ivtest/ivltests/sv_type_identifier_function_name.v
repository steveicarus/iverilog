// Check that function names can shadow visible type identifiers.

typedef int T;
typedef int U;

module test;

  reg failed;

  `define check(value, expected, error) \
    if ((value) !== (expected)) begin \
      $display("FAILED(%0d). %s", `__LINE__, error); \
      $display("  expected %0h, got %0h", expected, value); \
      failed = 1'b1; \
    end

  function int T(input int value);
    return value + 32'd1;
  endfunction

  function U;
    U = 1'b1;
  endfunction

  class C;
    function int T(input int value);
      return value + 32'd2;
    endfunction

    function int C(input int value);
      return value + 32'd3;
    endfunction
  endclass

  initial begin
    C c;

    failed = 1'b0;
    c = new;

    `check(T(32'd41), 32'd42, "Function name did not hide typedef");
    `check(U(), 1'b1, "Implicit function name did not hide typedef");
    `check(c.T(32'd40), 32'd42, "Class function name did not hide typedef");
    `check(c.C(32'd39), 32'd42, "Class function name did not hide class name");

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
