// Limit quantization uses each declaring module's precision.
`timescale 1ns/1ps
module period_fine #(parameter real LIMIT=0.050)(input wire CLK);
  reg notifier=0;
  specify $period(posedge CLK,LIMIT,notifier); endspecify
endmodule
`timescale 1ns/10ps
module period_coarse(input wire CLK);
  reg notifier=0;
  specify
    specparam LIMIT=0.056; // 56ps rounds to 60ps in this module.
    $period(posedge CLK,LIMIT,notifier);
  endspecify
endmodule
`timescale 1ps/1ps
module tb;
  reg CLK=0;
  period_fine fine(CLK); period_coarse coarse(CLK);
  period_history_check #(1,149) check_fine(fine.notifier);
  period_history_check #(2,149,199) check_coarse(coarse.notifier);
  initial begin
    #100 CLK=1; #1 CLK=0;
    #48 CLK=1; #1 CLK=0; // 49ps: both violate.
    #49 CLK=1; #1 CLK=0; // 50ps: fine equal, coarse violates.
    #59 CLK=1; #1 CLK=0; // 60ps: coarse equal.
    #60 CLK=1; #1 CLK=0; // 61ps: both safe.
    #10; check_fine.complete; check_coarse.complete;
    $display("PASSED"); $finish;
  end
endmodule
`include "period_history_check.vh"
