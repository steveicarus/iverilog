// An X constant is outside the supported condition forms.
`timescale 1ps/1ps
module tb;
  wire CLK, EN; reg notifier;
  specify
    $width(posedge CLK &&& (EN == 1'bx), 10, 0, notifier);
  endspecify
endmodule
