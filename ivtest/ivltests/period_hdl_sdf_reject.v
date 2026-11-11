`timescale 1ps/1ps
module period_reject_leaf(input wire CLK);
  wire en=1; wire OTHER=CLK;
  reg notifier=0;
  specify $period(posedge CLK,10,notifier); endspecify
endmodule
module tb;
  reg CLK=0;
  period_reject_leaf conditional(CLK);
  period_history_check #(1,124) check_conditional(conditional.notifier);
  period_reject_leaf malformed(CLK);
  period_history_check #(1,124) check_malformed(malformed.notifier);
  period_reject_leaf wrong_edge(CLK);
  period_history_check #(1,124) check_wrong_edge(wrong_edge.notifier);
  period_reject_leaf wrong_ref(CLK);
  period_history_check #(1,124) check_wrong_ref(wrong_ref.notifier);
  period_reject_leaf negative(CLK);
  period_history_check #(1,124) check_negative(negative.notifier);
  period_reject_leaf negative_magnitude(CLK);
  period_history_check #(1,124) check_negative_magnitude(negative_magnitude.notifier);
  period_reject_leaf overflow(CLK);
  period_history_check #(1,124) check_overflow(overflow.notifier);
  period_reject_leaf wrong_cell(CLK);
  period_history_check #(1,124) check_wrong_cell(wrong_cell.notifier);
  period_reject_leaf valid(CLK);
  period_history_check #(2,115,124) check_valid(valid.notifier);
  initial begin
    #5; $sdf_annotate("ivltests/period_hdl_sdf_reject.sdf",tb);
    $period_limit("tb.conditional","period","CLK","posedge",10.0,10);
    $period_limit("tb.malformed","period","CLK","posedge",10.0,10);
    $period_limit("tb.wrong_edge","period","CLK","posedge",10.0,10);
    $period_limit("tb.wrong_ref","period","CLK","posedge",10.0,10);
    $period_limit("tb.negative","period","CLK","posedge",10.0,10);
    $period_limit("tb.negative_magnitude","period","CLK","posedge",10.0,10);
    $period_limit("tb.overflow","period","CLK","posedge",10.0,10);
    $period_limit("tb.wrong_cell","period","CLK","posedge",10.0,10);
    $period_limit("tb.valid","period","CLK","posedge",20.0,20);
    #95 CLK=1; #1 CLK=0;
    #14 CLK=1; #1 CLK=0;
    #8 CLK=1; #1 CLK=0;
    #19 CLK=1; #1 CLK=0;
    #10;
    check_conditional.complete;
    check_malformed.complete;
    check_wrong_edge.complete;
    check_wrong_ref.complete;
    check_negative.complete;
    check_negative_magnitude.complete;
    check_overflow.complete;
    check_wrong_cell.complete;
    check_valid.complete;
    $display("PASSED");$finish;
  end
endmodule
`include "period_history_check.vh"
