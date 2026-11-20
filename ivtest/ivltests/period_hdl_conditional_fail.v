// Z constants and logical negation are outside the supported conditions.
`timescale 1ps/1ps
module tb;
  wire CLK,EN;reg notifier;
  specify
    $period(posedge CLK &&& (EN != 1'bz),10,notifier);
    $period(posedge CLK &&& !EN,10,notifier);
  endspecify
endmodule
