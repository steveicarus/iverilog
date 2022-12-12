// Check that it is possible to overwrite type parameters and that the provided
// type is evaluated in the scope instantiating the module.

bit failed = 1'b0;

`define check(expr, val) \
  if (expr != val) begin \
    $display("failed: %s, expected %0d, got %0d", `"expr`", val, expr); \
    failed = 1'b1; \
  end

module M #(
  parameter type T1 = integer,
  parameter WIDTH = 0
);
  typedef logic [1:0] T2;
  localparam A = 2;
  T1 x;

  initial begin
    `check($bits(x), WIDTH)
    `check($bits(T1), WIDTH)
  end

endmodule

module test;
  localparam A = 4;
  typedef logic [A-1:0] T2;

  M #(
    .WIDTH ($bits(integer))
  ) i_m1 ();

  M #(
    .T1 (logic [15:0]),
    .WIDTH (16)
  ) i_m2 ();

  // `A` must be evauluated in this context
  M #(
    .T1 (logic [A-1:0]),
    .WIDTH (4)
  ) i_m3 ();

  // `T2` must be evauluated in this context
  M #(
    .T1 (T2),
    .WIDTH (4)
  ) i_m4 ();

  initial begin
    #1
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
