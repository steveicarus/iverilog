`timescale 1ps/1ps
module tb;
  wire CLK, EN; reg notifier;
  specify
    $width(posedge CLK &&& EN, 10, 0, notifier);
  endspecify
endmodule
