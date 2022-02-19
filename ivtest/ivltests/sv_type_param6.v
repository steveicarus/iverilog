// Check that various syntax variations for type parameters without a default
// value are supported.

bit failed = 1'b0;

`define check(expr, val) \
  if (expr != val) begin \
    $display("failed: %s, expected %0d, got %0d", `"expr`", val, expr); \
    failed = 1'b1; \
  end

module M1 #(
  parameter type T1
);
  T1 x;

  initial begin
    `check($bits(x), $bits(integer))
    `check($bits(T1), $bits(integer))
  end

endmodule

module M2 #(
  type T1
);
  T1 x;

  initial begin
    `check($bits(x), $bits(integer))
    `check($bits(T1), $bits(integer))
  end

endmodule

module M3 #(
  parameter type T1, T2
);
  T1 x;
  T2 y = 1.23;

  initial begin
    `check($bits(x), $bits(integer))
    `check($bits(T1), $bits(integer))
    `check(y, 1.23)
  end

endmodule

module M4 #(
  type T1, T2
);
  T1 x;
  T2 y = 1.23;

  initial begin
    `check($bits(x), $bits(integer))
    `check($bits(T1), $bits(integer))
    `check(y, 1.23)
  end

endmodule

module test;

  M1 #(
    .T1 (integer)
  ) i_m1 ();

  M2 #(
    .T1 (integer)
  ) i_m2 ();

  M3 #(
    .T1 (integer),
    .T2 (real)
  ) i_m3();

  M4 #(
    .T1 (integer),
    .T2 (real)
  ) i_m4 ();

  initial begin
    #1
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
