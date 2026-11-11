`timescale 1ps/1ps
module leaf_default_leaf(input wire CLK, input wire en);
  reg notifier = 0;
  specify
    $width(posedge CLK, 10, 0, notifier);
  endspecify
endmodule
module leaf_annotated(input wire CLK, input wire en);
  reg notifier = 0;
  specify
    $width(posedge CLK, 10, 0, notifier);
  endspecify
endmodule
module tb;
  width_history_check #(1, 209, 0) check0(default_leaf.notifier);
  width_history_check #(2, 115, 209) check1(annotated.notifier);
  reg en = 1;
  reg clk_A = 0;
  leaf_default_leaf default_leaf(clk_A, en);
  leaf_annotated annotated(clk_A, en);
  initial begin
    #100 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #15 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #85 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #9 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #91 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #20 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
  end
  initial begin
    #350;
    $display("B_FINAL tb.default_leaf.notifier %b", default_leaf.notifier);
    $display("B_FINAL tb.annotated.notifier %b", annotated.notifier);
    check0.complete;
    check1.complete;
    $display("PASSED");
    $display("B_DONE W_SDF %0t", $time);
    $finish;
  end
  initial begin #5; $sdf_annotate("ivltests/width_hdl.sdf", annotated); end
endmodule

`include "width_history_check.vh"
