// Proposed qualified-history semantics; expectations are source-derived.
`timescale 1ps/1ps
module leaf_cp150(input wire CLK, input wire en);
  reg notifier = 0;
  initial #1 $pulse_watch(notifier, 3, 150, 1, 400);
  specify
    $period(negedge CLK &&& (en === 1'b1), 150, notifier);
  endspecify
endmodule
module leaf_cp250(input wire CLK, input wire en);
  reg notifier = 0;
  initial #1 $pulse_watch(notifier, 3, 250, 2, 300, 400);
  specify
    $period(negedge CLK &&& (en === 1'b1), 250, notifier);
  endspecify
endmodule
module leaf_ctrl150(input wire CLK, input wire en);
  reg notifier = 0;
  initial #1 $pulse_watch(notifier, 3, 150, 3, 200, 300, 400);
  specify
    $period(negedge CLK, 150, notifier);
  endspecify
endmodule
module tb;
  reg en = 1;
  reg clk_A = 1;
  leaf_cp150 cp150(clk_A, en);
  leaf_cp250 cp250(clk_A, en);
  leaf_ctrl150 ctrl150(clk_A, en);
  initial begin
    #100 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #10 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #90 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #10 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #90 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #10 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #90 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #10 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #290 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #10 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
  end
  initial begin
    #150 en = 0;
    $display("B_DRIVE %0t en %b", $time, en);
    #100 en = 1;
    $display("B_DRIVE %0t en %b", $time, en);
    #200 en = 0;
    $display("B_DRIVE %0t en %b", $time, en);
    #50 en = 1;
    $display("B_DRIVE %0t en %b", $time, en);
  end
  initial begin
    #750;
    $pulse_complete(3);
    $finish;
  end
endmodule
