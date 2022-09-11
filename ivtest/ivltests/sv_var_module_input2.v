// Check that the var keyword is supported for module non-ANSI input ports

bit failed = 1'b0;

`define check(val, exp) \
  if (val !== exp) begin \
    $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, `"val`", val, exp); \
    failed = 1'b1; \
  end

module M(x, y, z, w);
  parameter VAL_X = 0;
  parameter VAL_Y = 0;
  parameter VAL_Z = 0;
  parameter VAL_W = 0;

  input var x;
  input var [7:0] y;
  input var signed [7:0] z;
  input var logic [7:0] w;

  initial begin
    `check(x, VAL_X)
    `check(y, VAL_Y)
    `check(z, VAL_Z)
    `check(w, VAL_W)
  end

endmodule

module test;

  M #(
    .VAL_X (1'b1),
    .VAL_Y (8'd10),
    .VAL_Z (-8'sd1),
    .VAL_W (8'd20)
  ) i_m1 (
    .x (1'b1),
    .y (8'd10),
    .z (-8'sd1),
    .w (8'd20)
  );

  // When unconnected it should default to x, rather z
  M #(
    .VAL_X (1'bx),
    .VAL_Y (8'hx),
    .VAL_Z (8'hx),
    .VAL_W (8'hx)
  ) i_m2 ();

  initial begin
    #1
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
