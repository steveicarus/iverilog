// Check that the var keyword is supported for module ANSI output ports

bit failed = 1'b0;

`define check(val, exp) \
  if (val !== exp) begin \
    $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, `"val`", val, exp); \
    failed = 1'b1; \
  end

module M #(
  parameter VAL_X = 0,
  parameter VAL_Y = 0,
  parameter VAL_Z = 0,
  parameter VAL_W = 0
) (
  output var x,
  output var [7:0] y,
  output var signed [7:0] z,
  output var logic [7:0] w
);

  assign x = VAL_X;
  assign y = VAL_Y;
  assign z = VAL_Z;
  assign w = VAL_W;

endmodule

module test;

  logic x1;
  logic x2;
  logic [7:0] y1;
  logic [7:0] y2;
  logic signed [7:0] z1;
  logic signed [7:0] z2;
  logic [7:0] w1;
  logic [7:0] w2;

  M #(
    .VAL_X (1'b1),
    .VAL_Y (10),
    .VAL_Z (-1),
    .VAL_W (20)
  ) i_m1 (
    .x (x1),
    .y (y1),
    .z (z1),
    .w (w1)
  );

  // The type for var should default to logic, check that the value can be X
  M #(
    .VAL_X (1'bx),
    .VAL_Y (8'hxx),
    .VAL_Z (8'hxx),
    .VAL_W (8'hxx)
  ) i_m2 (
    .x (x2),
    .y (y2),
    .z (z2),
    .w (w2)
  );

  initial begin
    `check(x1, 1'b1)
    `check(y1, 10)
    `check(z1, -1)
    `check(w1, 20)
    `check(x2, 1'bx)
    `check(y2, 8'hxx)
    `check(z2, 8'hxx)
    `check(w2, 8'hxx)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
