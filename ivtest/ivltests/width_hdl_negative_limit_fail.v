`timescale 1ps/1ps
module tb;
  wire CLK; reg notifier;
  specify
    $width(posedge CLK, -1, 0, notifier);
  endspecify
endmodule
