`timescale 1ps/1ps
module tb;
  wire CLK;reg notifier;
  specify $period(CLK,10,notifier); endspecify
endmodule
