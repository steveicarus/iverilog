`timescale 1ps/1ps
module leaf_a(input wire CLK, input wire en);
  reg notifier = 0;
  specify
    $width(posedge CLK, 10, 0, notifier);
  endspecify
endmodule
module leaf_b(input wire CLK, input wire en);
  reg notifier = 0;
  specify
    $width(posedge CLK, 10, 0, notifier);
  endspecify
endmodule
module leaf_zebra(input wire CLK, input wire en);
  reg notifier = 0;
  specify
    $width(posedge CLK, 10, 0, notifier);
  endspecify
endmodule
module tb;
  width_history_check #(1, 109, 0) check0(a.notifier);
  width_history_check #(1, 212, 0) check1(b.notifier);
  width_history_check #(1, 109, 0) check2(zebra.notifier);
  reg en = 1;
  reg clk_A = 0;
  reg clk_B = 0;
  leaf_a a(clk_A, en);
  leaf_b b(clk_B, en);
  leaf_zebra zebra(clk_A, en);
  initial begin
    #100 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #9 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #91 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #10 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #90 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #11 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
  end
  initial begin
    #103 clk_B = 1;
    $display("B_DRIVE %0t clk_B %b", $time, clk_B);
    #11 clk_B = 0;
    $display("B_DRIVE %0t clk_B %b", $time, clk_B);
    #89 clk_B = 1;
    $display("B_DRIVE %0t clk_B %b", $time, clk_B);
    #9 clk_B = 0;
    $display("B_DRIVE %0t clk_B %b", $time, clk_B);
    #91 clk_B = 1;
    $display("B_DRIVE %0t clk_B %b", $time, clk_B);
    #10 clk_B = 0;
    $display("B_DRIVE %0t clk_B %b", $time, clk_B);
  end
  initial begin
    #350;
    $display("B_FINAL tb.a.notifier %b", a.notifier);
    $display("B_FINAL tb.b.notifier %b", b.notifier);
    $display("B_FINAL tb.zebra.notifier %b", zebra.notifier);
    check0.complete;
    check1.complete;
    check2.complete;
    $display("PASSED");
    $display("B_DONE W %0t", $time);
    $finish;
  end
endmodule

`include "width_history_check.vh"
