// Proposed qualified-history semantics; expectations are source-derived.
`timescale 1ps/1ps
module leaf_cf100(input wire CLK, input wire en);
  reg notifier = 0;
  initial #1 $pulse_watch(notifier, 4, 100, 1, 209);
  specify
    $width(negedge CLK &&& (en === 1'b1), 100, 0, notifier);
  endspecify
endmodule
module leaf_ctrl100(input wire CLK, input wire en);
  reg notifier = 0;
  initial #1 $pulse_watch(notifier, 4, 100, 2, 120, 209);
  specify
    $width(negedge CLK, 100, 0, notifier);
  endspecify
endmodule
module tb;
  reg en = 0;
  reg clk_A = 1;
  leaf_cf100 cf100(clk_A, en);
  leaf_ctrl100 ctrl100(clk_A, en);
  initial begin
    #100 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #20 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #80 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #9 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
  end
  initial begin
    #110 en = 1;
    $display("B_DRIVE %0t en %b", $time, en);
  end
  initial begin
    #250;
    $pulse_complete(2);
    $finish;
  end
endmodule
