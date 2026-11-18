`timescale 1ps/1ps
module tb;
  wire [1:0] CLK; reg notifier;
  specify
    $width(posedge CLK, 10, 0, notifier);
  endspecify
endmodule
