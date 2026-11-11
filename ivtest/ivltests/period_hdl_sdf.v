`timescale 1ps/1ps
module period_sdf_leaf(input wire CLK);
  reg notifier=0;
  specify $period(posedge CLK,10,notifier); endspecify
endmodule
`timescale 1ps/1ps
module tb;
  reg CLK=0;
  period_sdf_leaf default_leaf(CLK), annotated(CLK);
  period_history_check #(1,124) check_default(default_leaf.notifier);
  period_history_check #(2,115,124) check_annotated(annotated.notifier);
  initial begin
    #5; $sdf_annotate("ivltests/period_hdl.sdf", annotated);
    $period_limit("tb.annotated","period","CLK","posedge",20.0,20);
    #95 CLK=1; #1 CLK=0;
    #14 CLK=1; #1 CLK=0;
    #8 CLK=1; #1 CLK=0;
    #19 CLK=1; #1 CLK=0;
    #10;
    check_default.complete;check_annotated.complete;
    $display("PASSED"); $finish;
  end
endmodule
`include "period_history_check.vh"
