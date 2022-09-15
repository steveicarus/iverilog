// Check that forward enum typedefs are supported

module test;

`define check(val, exp) \
  if (val !== exp) begin \
    $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, `"val`", exp, val); \
    failed = 1'b1; \
  end

  bit failed = 1'b0;

  typedef T;
  typedef enum T;

  T x;

  typedef enum integer {
    A, B
  } T;

  T y;

  typedef enum T;
  typedef T;

  T z;

  initial begin
    // Check that they are all the same type and can be assigned to each other
    x = y;
    y = z;
    z = x;

    `check($bits(x), $bits(integer))
    `check($bits(y), $bits(integer))
    `check($bits(z), $bits(integer))

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
