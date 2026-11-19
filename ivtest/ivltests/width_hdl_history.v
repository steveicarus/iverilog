// No prior opening is different from a real reference event at time zero.
`timescale 1ps/1ps
module late_open(input wire CLK);
  reg notifier = 0;
  specify $width(posedge CLK, 30, 0, notifier); endspecify
endmodule
module zero_open(input wire CLK);
  reg notifier = 0;
  specify $width(posedge CLK, 10, 0, notifier); endspecify
endmodule
module tb;
  reg late_clk;
  reg zero_clk;
  late_open late(late_clk);
  zero_open zero(zero_clk);
  width_history_check #(1, 109) check_late(late.notifier);
  width_history_check #(1, 9) check_zero(zero.notifier);
  initial begin
    #20 late_clk = 0; // X->0 is a close without a matching opening.
    #80 late_clk = 1;
    #9 late_clk = 0;
  end
  initial begin
    zero_clk = 1; // The genuine X->1 reference is at time zero.
    #9 zero_clk = 0;
  end
  initial begin
    #150;
    check_late.complete;
    check_zero.complete;
    $display("PASSED");
    $finish;
  end
endmodule
`include "width_history_check.vh"
