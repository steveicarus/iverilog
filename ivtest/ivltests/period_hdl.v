`timescale 1ps/1ps
module leaf_a(input wire CLK, input wire en);
  reg notifier = 0;
  specify
    $period(posedge CLK, 50, notifier);
  endspecify
endmodule
module leaf_b(input wire CLK, input wire en);
  reg notifier = 0;
  specify
    $period(posedge CLK, 60, notifier);
  endspecify
endmodule
module leaf_zebra(input wire CLK, input wire en);
  reg notifier = 0;
  specify
    $period(posedge CLK, 50, notifier);
  endspecify
endmodule
module tb;
  period_history_check #(2,69,219) check_a(a.notifier);
  period_history_check #(1,143) check_b(b.notifier);
  period_history_check #(2,69,219) check_zebra(zebra.notifier);
  reg en = 1;
  reg clk_A = 0;
  reg clk_B = 0;
  leaf_a a(clk_A, en);
  leaf_b b(clk_B, en);
  leaf_zebra zebra(clk_A, en);
  initial begin
    #20 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #10 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #39 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #10 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #40 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #10 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #41 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #10 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #39 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #10 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
  end
  initial begin
    #23 clk_B = 1;
    $display("B_DRIVE %0t clk_B %b", $time, clk_B);
    #10 clk_B = 0;
    $display("B_DRIVE %0t clk_B %b", $time, clk_B);
    #51 clk_B = 1;
    $display("B_DRIVE %0t clk_B %b", $time, clk_B);
    #10 clk_B = 0;
    $display("B_DRIVE %0t clk_B %b", $time, clk_B);
    #49 clk_B = 1;
    $display("B_DRIVE %0t clk_B %b", $time, clk_B);
    #10 clk_B = 0;
    $display("B_DRIVE %0t clk_B %b", $time, clk_B);
    #50 clk_B = 1;
    $display("B_DRIVE %0t clk_B %b", $time, clk_B);
    #10 clk_B = 0;
    $display("B_DRIVE %0t clk_B %b", $time, clk_B);
  end
  initial begin
    #230;
    $display("B_FINAL tb.a.notifier %b", a.notifier);
    $display("B_FINAL tb.b.notifier %b", b.notifier);
    $display("B_FINAL tb.zebra.notifier %b", zebra.notifier);
    $display("B_DONE P %0t", $time);
    check_a.complete; check_b.complete; check_zebra.complete;
    $display("PASSED");
    $finish;
  end
endmodule

`include "period_history_check.vh"
