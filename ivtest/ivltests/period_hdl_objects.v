// Same-scope types and optional notifiers must retain distinct objects.
`timescale 1ps/1ps
module period_same_scope(input wire CLK);
  reg n10=0,n20=0,nw=0;
  specify
    $period(posedge CLK,10,n10);
    $period(posedge CLK,20,n20);
    $width(posedge CLK,5,0,nw);
  endspecify
endmodule
module period_omit(input wire CLK);
  specify $period(posedge CLK,10); endspecify
endmodule
module period_zero(input wire CLK);
  specify $period(posedge CLK,0); endspecify
endmodule
module tb;
  reg CLK=0;
  period_same_scope same(CLK); period_omit omit(CLK); period_zero zero(CLK);
  period_history_check #(1,109) check10(same.n10);
  period_history_check #(2,109,119) check20(same.n20);
  period_history_check #(1,102) checkw(same.nw);
  initial begin #1; $period_objects; end
  initial begin
    #100 CLK=1; #2 CLK=0;
    #7 CLK=1; #5 CLK=0;
    #5 CLK=1; #6 CLK=0;
    #15 CLK=1; #5 CLK=0;
    #10; check10.complete;check20.complete;checkw.complete;
    $period_objects_finish;
    $display("PASSED"); $finish;
  end
endmodule
`include "period_history_check.vh"
