`timescale 1ps/1ps
module leaf_a(input wire CLK, input wire en);
  reg notifier = 0;
  specify
    $width(posedge CLK, 10, 4, notifier);
  endspecify
endmodule
module tb;
  width_history_check #(2, 305, 409) check0(a.notifier);
  reg en = 1;
  reg clk_A = 0;
  leaf_a a(clk_A, en);
  initial begin
    #100 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #3 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #97 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #4 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #96 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #5 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #95 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #9 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #91 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #10 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
  end
  initial begin
    #550;
    $display("B_FINAL tb.a.notifier %b", a.notifier);
    check0.complete;
    $display("PASSED");
    $display("B_DONE WT %0t", $time);
    $finish;
  end
endmodule

`include "width_history_check.vh"
