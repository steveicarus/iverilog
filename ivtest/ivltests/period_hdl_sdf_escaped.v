`timescale 1ps/1ps
module period_dot(input wire \CLK.dot );
  reg notifier=0;
  specify $period(posedge \CLK.dot ,10,notifier); endspecify
endmodule
module period_backslash(input wire \CLK\tail );
  reg notifier=0;
  specify $period(posedge \CLK\tail ,10,notifier); endspecify
endmodule
module period_nested(input wire CLK); endmodule
module period_hierarchy(input wire CLK);
  reg notifier=0;period_nested nested(CLK);
  specify $period(posedge CLK,10,notifier); endspecify
endmodule
module tb;
  reg CLK=0;
  period_dot dotted(CLK);period_backslash slashed(CLK);period_hierarchy hierarchy(CLK);
  period_history_check #(2,115,124) check0(dotted.notifier);
  period_history_check #(2,115,124) check1(slashed.notifier);
  period_history_check #(1,124) check2(hierarchy.notifier);
  initial begin
    #5;$sdf_annotate("ivltests/period_hdl_sdf_escaped.sdf",tb);
    $period_limit("tb.dotted","period","CLK.dot","posedge",20.0,20);
    $period_limit("tb.slashed","period","CLK\\tail","posedge",20.0,20);
    $period_limit("tb.hierarchy","period","CLK","posedge",10.0,10);
    #95 CLK=1; #1 CLK=0;
    #14 CLK=1; #1 CLK=0;
    #8 CLK=1; #1 CLK=0;
    #19 CLK=1; #1 CLK=0;
    #10;
    check0.complete;check1.complete;check2.complete;
    $display("PASSED");$finish;
  end
endmodule
`include "period_history_check.vh"
