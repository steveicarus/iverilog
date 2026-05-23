// Check that restricted enum type parameter overrides are supported.

typedef enum bit [2:0] {
  A0, B0
} T0;

typedef enum bit [3:0] {
  A1, B1
} T1;

module M #(
  parameter type enum T = T0
);
  T x;
endmodule

module test;

`define check(val, exp) \
  if (val !== exp) begin \
    $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, `"val`", exp, val); \
    failed = 1'b1; \
  end

  bit failed = 1'b0;

  M #(
    .T(T1)
  ) i_m();

  initial begin
    `check($bits(i_m.x), $bits(T1))

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
