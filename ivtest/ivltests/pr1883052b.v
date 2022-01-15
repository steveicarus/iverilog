`timescale 1us/100ns

module top;
  reg pass = 1'b1;

  reg [3:0] ia = 4'd1;
  wire signed [3:0] iconstp, iconstm, isfunc, iufunc;
  wire [7:0] istring;

  /* Integer constant value. */
  assign #1 iconstp = 2; // 2
  assign #1 iconstm = -2; // -2
  assign #1 istring = "0"; // "0"

  /* Integer System Function. */
  assign #1 isfunc = $rtoi(2.0); // 2

  /* Integer User Function. */
  assign #1 iufunc = int_func(ia); // 2

  initial begin
//    $monitor($realtime,, iconstp,, iconstm,, istring,, iufunc,, isfunc);
    #0.9;
    if (iconstp !== 4'bx) begin
      pass = 1'b0;
      $display("Integer: constant (positive) value not delayed.");
    end
    if (iconstm !== 4'bx) begin
      pass = 1'b0;
      $display("Integer: constant (negative) value not delayed.");
    end
    if (istring !== 8'bx) begin
      pass = 1'b0;
      $display("Integer: string value not delayed.");
    end
    if (isfunc !== 4'bx) begin
      pass = 1'b0;
      $display("Integer: system function value not delayed.");
    end
    if (iufunc !== 4'bx) begin
      pass = 1'b0;
      $display("Integer: user function value not delayed.");
    end

    #0.1;
    #0;
    if (iconstp !== 2) begin
      pass = 1'b0;
      $display("Integer: constant (positive) value not delayed correctly.");
    end
    if (iconstm !== -2) begin
      pass = 1'b0;
      $display("Integer: constant (negative) value not delayed correctly.");
    end
    if (istring !== "0") begin
      pass = 1'b0;
      $display("Integer: string value not delayed correctly.");
    end
    if (isfunc !== 2) begin
      pass = 1'b0;
      $display("Integer: system function value not delayed correctly.");
    end
    if (iufunc !== 2) begin
      pass = 1'b0;
      $display("Integer: user function value not delayed correctly.");
    end

    if (pass) $display("PASSED");
  end

  function [31:0] int_func;
    input [31:0] in;
    int_func = in * 2;
  endfunction

endmodule
