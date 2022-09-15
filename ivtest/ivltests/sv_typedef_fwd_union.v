// Check that forward typdes of unions are supported

module test;

`define check(val, exp) \
  if (val !== exp) begin \
    $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, `"val`", exp, val); \
    failed = 1'b1; \
  end

  bit failed = 1'b0;
  typedef union T;
  typedef T;

  T x;

  typedef union packed {
    int x;
    logic [3:0][7:0] y;
  } T;

  T y;

  // There can be as many forward typedefs as we like, even after the type
  // itself has already been declared.
  typedef T;
  typedef union T;

  T z;

  initial begin
    `check($bits(x), $bits(int));
    `check($bits(y), $bits(int));
    `check($bits(z), $bits(int));

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
