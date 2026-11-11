// Rejected annotations preserve actual typed WIDTH objects and limits.
`timescale 1ps/1ps
module width_reject_leaf(input wire CLK, input wire en);
  reg notifier = 0;
  specify
    $width(posedge CLK, 10, 0, notifier);
  endspecify
endmodule
module tb;
  reg CLK=0;
  width_reject_leaf conditional(CLK, 1'b1);
  width_reject_leaf malformed(CLK, 1'b1);
  width_reject_leaf wrong_edge(CLK, 1'b1);
  width_reject_leaf overflow(CLK, 1'b1);
  width_history_check #(0) check0(conditional.notifier);
  width_history_check #(0) check1(malformed.notifier);
  width_history_check #(0) check2(wrong_edge.notifier);
  width_history_check #(0) check3(overflow.notifier);
  initial begin
    #5;
    $sdf_annotate("ivltests/width_hdl_sdf_negative.sdf", tb);
    $width_rejected_limits;
    #95 CLK=1;
    #15 CLK=0;
    #10;
    check0.complete;
    check1.complete;
    check2.complete;
    check3.complete;
    $display("PASSED");
    $finish;
  end
endmodule
`include "width_history_check.vh"
