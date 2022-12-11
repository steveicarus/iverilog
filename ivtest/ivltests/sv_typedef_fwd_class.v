// Check that forward typedefs of classes are supported

module test;

`define check(val, exp) \
  if (val !== exp) begin \
    $display("FAILED(%0d). '%s' expected %d, got %d", `__LINE__, `"val`", exp, val); \
    failed = 1'b1; \
  end

  bit failed = 1'b0;

  typedef class C;
  typedef C;

  C x;

  class C;
    int x;
  endclass

  C y;

  // There can be as many forward typedefs as we like, even after the type
  // itself has already been declared.
  typedef C;
  typedef class C;

  C z;

  initial begin
    // Check they are all the same type and can be assigned to each other
    x = y;
    y = z;
    z = x;

    `check($bits(x.x), $bits(int));
    `check($bits(y.x), $bits(int));
    `check($bits(z.x), $bits(int));

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
