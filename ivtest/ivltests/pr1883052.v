`timescale 1us/100ns

module top;
  reg pass = 1'b1;

  reg [3:0] ia = 4'd1, ib = 4'd2;
  wire signed [3:0] icon;

  assign #1 icon = {ib[1:0], ia[0]}; // Should give 5 after a delay.

  initial begin
    #0.9;
    if (icon !== 4'bx) begin
      pass = 1'b0;
      $display("concatenation value not delayed, expected 4'bx got %b.", icon);
    end

    #0.1;
    #0;
    if (icon !== 4'd5) begin
      pass = 1'b0;
      $display("concatenation value not correct, expected 4'd5 got %d.", icon);
    end

    if (pass) $display("PASSED");
  end
endmodule
