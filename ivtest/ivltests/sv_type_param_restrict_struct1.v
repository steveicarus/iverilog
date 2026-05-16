// Check that restricted struct type parameter defaults are supported.

typedef struct packed {
  logic [3:0] x;
} T0;

module M #(
  parameter type struct T = T0
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

  M i_m();

  initial begin
    `check($bits(i_m.x), $bits(T0))

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
