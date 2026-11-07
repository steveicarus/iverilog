`timescale 1ps/1ps
module period_child(input wire CLK);endmodule
module tb;
  wire CLK;reg notifier;period_child child(CLK);
  specify $period(posedge child.CLK,10,notifier); endspecify
endmodule
