// Check that forward struct typedefs are supported

module test;

`define check(val, exp) \
  if (val !== exp) begin \
    $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, `"val`", exp, val); \
    failed = 1'b1; \
  end

  bit failed = 1'b0;

  typedef T;
  typedef struct T;

  T x;

  typedef struct packed {
    int x;
  } T;

  T y;

  typedef struct T;
  typedef T;

  T z;

  initial begin
    `check($bits(x), $bits(int))
    `check($bits(y), $bits(int))
    `check($bits(z), $bits(int))

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
