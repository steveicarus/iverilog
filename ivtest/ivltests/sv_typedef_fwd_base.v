// Check that forward typedefs of basic types are supported

`define check(val, exp) \
  if (val != exp) begin \
    $display("FAILED(%0d). '%s' expected ", `__LINE__, `"val`", exp, " got ", val, ); \
    failed = 1'b1; \
  end

bit failed = 1'b0;

module test;
  typedef T1;
  typedef T1; // Check forward typedef twice for the same type
  typedef T2;
  typedef T3;
  typedef T4;

  T1 x = -1;
  T2 y = 1.23;
  T3 z = "Hello";
  T4 w;

  typedef integer T1;
  // There can be as many forward typedefs as we like, even after the type
  // itself has already been declared.
  typedef T1;
  typedef T1;

  typedef real T2;
  typedef string T3;
  typedef logic [1:0] T4[3:0];

  initial begin
    `check($bits(x), $bits(integer))
    `check($bits(T1), $bits(integer))
    `check(x, -1)
    `check(y, 1.23)
    `check(z, "Hello")
    `check($unpacked_dimensions(w), 1)
    `check($size(w), 4)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
