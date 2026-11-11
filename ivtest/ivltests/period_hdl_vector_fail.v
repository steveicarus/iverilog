`timescale 1ps/1ps
module tb;
  wire [1:0] CLK;reg notifier;
  specify $period(posedge CLK,10,notifier); endspecify
endmodule
