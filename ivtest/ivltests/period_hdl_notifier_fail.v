`timescale 1ps/1ps
module tb;
  wire CLK;wire notifier;
  specify $period(posedge CLK,10,notifier); endspecify
endmodule
