// Physical stimulus is in ps; declaration values use each leaf's own units.
`timescale 1ns/1ps
module fine_width(input wire CLK);
  reg notifier = 0;
  specify
    specparam LIMIT = 0.010, THRESHOLD = 0.004;
    $width(posedge CLK, LIMIT, THRESHOLD, notifier);
  endspecify
endmodule
`timescale 1ns/10ps
module coarse_width(input wire CLK);
  reg notifier = 0;
  specify
    // Quantized at this module's precision: limit30ps, threshold10ps.
    specparam LIMIT = 0.026, THRESHOLD = 0.014;
    $width(posedge CLK, LIMIT, THRESHOLD, notifier);
  endspecify
endmodule
`timescale 1ps/1ps
module tb;
  reg fine_clk = 0, coarse_clk = 0;
  fine_width fine(fine_clk);
  coarse_width coarse(coarse_clk);
  width_history_check #(2, 305, 409) check_fine(fine.notifier);
  width_history_check #(2, 311, 429) check_coarse(coarse.notifier);
  initial begin
    #100 fine_clk = 1;
    #3 fine_clk = 0;
    #97 fine_clk = 1;
    #4 fine_clk = 0;
    #96 fine_clk = 1;
    #5 fine_clk = 0;
    #95 fine_clk = 1;
    #9 fine_clk = 0;
    #91 fine_clk = 1;
    #10 fine_clk = 0;
  end
  initial begin
    #100 coarse_clk = 1;
    #9 coarse_clk = 0;
    #91 coarse_clk = 1;
    #10 coarse_clk = 0;
    #90 coarse_clk = 1;
    #11 coarse_clk = 0;
    #89 coarse_clk = 1;
    #29 coarse_clk = 0;
    #71 coarse_clk = 1;
    #30 coarse_clk = 0;
  end
  initial begin
    #550;
    check_fine.complete;
    check_coarse.complete;
    $display("PASSED");
    $finish;
  end
endmodule
`include "width_history_check.vh"
