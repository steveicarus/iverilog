`timescale 1ps/1ps
module tb;
  wire CLK;reg notifier;
  specify $period(posedge CLK,-1,notifier); endspecify
endmodule
