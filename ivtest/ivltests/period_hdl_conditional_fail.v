// A Z constant is outside the supported condition forms.
`timescale 1ps/1ps
module tb;
  wire CLK,EN;reg notifier;
  specify $period(posedge CLK &&& (EN != 1'bz),10,notifier); endspecify
endmodule
