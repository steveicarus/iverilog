// PERIOD annotation must not alter the WIDTH check on the same scalar edge.
`timescale 1ps/1ps
module period_mixed_sdf_leaf(input wire CLK);
  reg np=0,nw=0;
  specify
    $period(posedge CLK,10,np);
    $width(posedge CLK,5,0,nw);
  endspecify
endmodule
module tb;
  reg CLK=0;
  period_mixed_sdf_leaf dut(CLK);
  period_history_check #(2,115,124) checkp(dut.np);
  period_history_check #(1,102) checkw(dut.nw);
  initial begin
    #5;$sdf_annotate("ivltests/period_hdl_sdf_mixed.sdf",dut);
    $period_limit("tb.dut","period","CLK","posedge",20.0,20);
    $period_limit("tb.dut","width","CLK","posedge",5.0,5);
    #95 CLK=1;#2 CLK=0;
    #13 CLK=1;#5 CLK=0;
    #4 CLK=1;#6 CLK=0;
    #14 CLK=1;#5 CLK=0;
    #10;checkp.complete;checkw.complete;
    $display("PASSED");$finish;
  end
endmodule
`include "period_history_check.vh"
