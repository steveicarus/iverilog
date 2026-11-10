// Two distinct posedges at t=0 have period 0 < 10 ps: one violation.
// The next posedge at 21 ps is outside the forbidden interval.
`timescale 1ps/1ps
module startup_period(CLK, notifier);
  input CLK;
  output notifier;
  reg notifier;
  initial notifier = 0;
  specify $period(posedge CLK, 10, notifier); endspecify
endmodule
module tb;
  reg CLK;
  wire observed;
  startup_period dut(CLK, observed);
  period_history_check #(1, 0, 0) history(observed);
  initial begin
    CLK=0; #0 CLK=1; #0 CLK=0; #0 CLK=1;
    #20 CLK=0; #1 CLK=1;
    #5 history.complete;
    $display("PASSED"); $finish;
  end
endmodule
`include "period_history_check.vh"
