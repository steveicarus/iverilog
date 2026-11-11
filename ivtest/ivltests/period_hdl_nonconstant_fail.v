`timescale 1ps/1ps
module tb;
  wire CLK;integer limit=10;reg notifier;
  specify $period(posedge CLK,limit,notifier); endspecify
endmodule
