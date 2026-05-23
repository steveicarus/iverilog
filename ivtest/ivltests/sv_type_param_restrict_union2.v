// Check that restricted union type parameter overrides are supported.

typedef union packed {
  logic [3:0] x;
  logic [3:0] y;
} T0;

typedef union packed {
  logic [7:0] x;
  logic [7:0] y;
} T1;

module M #(
  parameter type union T = T0
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
