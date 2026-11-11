// Genuine reference at time zero is distinct from absent reference history.
`timescale 1ps/1ps
module period_history_leaf(input wire CLK);
  reg notifier=0;
  specify $period(posedge CLK,10,notifier); endspecify
endmodule
module tb;
  reg zero_clk, late_clk;
  period_history_leaf zero(zero_clk), late(late_clk);
  period_history_check #(1,9) check_zero(zero.notifier);
  period_history_check #(1,109) check_late(late.notifier);
  initial begin
    zero_clk=1; // Genuine X->1 reference at time zero.
    #1 zero_clk=0;
    #8 zero_clk=1; // 9 < 10: violation.
    #1 zero_clk=0;
    #9 zero_clk=1; // 19-9 == 10: safe; prior violation refreshed history.
  end
  initial begin
    #2 late_clk=0; // Opposite transition cannot manufacture reference history.
    #98 late_clk=1; // First reference: no comparison against time zero.
    #1 late_clk=0;
    #8 late_clk=1;
    #1 late_clk=0;
    #10 late_clk=1; // 120-109 == 11: safe.
  end
  initial begin
    #150; check_zero.complete; check_late.complete;
    $display("PASSED"); $finish;
  end
endmodule
`include "period_history_check.vh"
