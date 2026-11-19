// Proposed qualified-history semantics; expectations are source-derived.
`timescale 1ps/1ps
module leaf_cw100(input wire CLK, input wire en);
  reg notifier = 0;
  initial #1 $pulse_watch(notifier, 4, 100, 1, 160);
  specify
    $width(posedge CLK &&& (en === 1'b1), 100, 0, notifier);
  endspecify
endmodule
module leaf_cw200(input wire CLK, input wire en);
  reg notifier = 0;
  initial #1 $pulse_watch(notifier, 4, 200, 2, 160, 240);
  specify
    $width(posedge CLK &&& (en === 1'b1), 200, 0, notifier);
  endspecify
endmodule
module leaf_ctrl100(input wire CLK, input wire en);
  reg notifier = 0;
  initial #1 $pulse_watch(notifier, 4, 100, 3, 120, 160, 240);
  specify
    $width(posedge CLK, 100, 0, notifier);
  endspecify
endmodule
module tb;
  reg en = 1;
  reg clk_A = 0;
  leaf_cw100 cw100(clk_A, en);
  leaf_cw200 cw200(clk_A, en);
  leaf_ctrl100 ctrl100(clk_A, en);
  initial begin
    #100 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #20 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #20 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #20 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #40 clk_A = 1;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
    #40 clk_A = 0;
    $display("B_DRIVE %0t clk_A %b", $time, clk_A);
  end
  initial begin
    #110 en = 0;
    $display("B_DRIVE %0t en %b", $time, en);
    #40 en = 1;
    $display("B_DRIVE %0t en %b", $time, en);
    #30 en = 0;
    $display("B_DRIVE %0t en %b", $time, en);
    #40 en = 1;
    $display("B_DRIVE %0t en %b", $time, en);
  end
  initial begin
    #280;
    $pulse_complete(3);
    $finish;
  end
endmodule
