`timescale 1ps/1ps
module tb;
  wire CLK; reg notifier;
  specify
    $width(posedge CLK, 10, -1, notifier);
  endspecify
endmodule
