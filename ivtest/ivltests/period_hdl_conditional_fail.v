`timescale 1ps/1ps
module tb;
  wire CLK,EN;reg notifier;
  specify $period(posedge CLK &&& EN,10,notifier); endspecify
endmodule
