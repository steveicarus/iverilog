// Real negative zero is numerically zero, not an unsupported negative limit.
`timescale 1ps/1ps
module negative_zero_threshold(input wire CLK);
  reg notifier=0;
  specify
    $width(posedge CLK, 10, -0.0, notifier);
  endspecify
endmodule
module negative_zero_limit(input wire CLK);
  reg notifier=0;
  specify
    $width(posedge CLK, -0.0, 0, notifier);
  endspecify
endmodule
module tb;
  reg CLK=0;
  negative_zero_threshold threshold(CLK);
  negative_zero_limit lim(CLK);
  width_history_check #(1,109,0) check0(threshold.notifier);
  width_history_check #(0) check1(lim.notifier);
  initial begin
    #100 CLK=1;
    #9 CLK=0;
    #10;
    check0.complete;
    check1.complete;
    $display("PASSED");
    $finish;
  end
endmodule
`include "width_history_check.vh"
