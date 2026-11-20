// X constants and logical negation are outside the supported conditions.
`timescale 1ps/1ps
module tb;
  wire CLK, EN; reg notifier;
  specify
    $width(posedge CLK &&& (EN == 1'bx), 10, 0, notifier);
    $width(posedge CLK &&& !EN, 10, 0, notifier);
  endspecify
endmodule
