// Logical equality is outside this finite condition syntax.
`timescale 1ps/1ps
module tb;
  wire CLK,EN;reg notifier;
  specify $period(posedge CLK &&& (EN == 1'b1),10,notifier); endspecify
endmodule
