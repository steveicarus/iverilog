// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module rrc_precision(R,C);
 input R,C;
 specify $recrem(posedge R,posedge C,1,1); endspecify
endmodule
module tb;
 reg R,C;
 rrc_precision recovery_only(R,C),removal_only(R,C);
 rrc_precision omitted_recovery(R,C),omitted_removal(R,C);
 initial begin
   R=0;C=0;
   #1;
   // 0x00200000:00000001 is 2^53+1 ticks: not exactly a double.
   // Pass high/low words as integer arguments; no real-valued oracle.
   $rrc_exact_set("tb.recovery_only",0,1,32'h00200000,1);
   $rrc_exact_set("tb.removal_only",32'h00200000,3,0,1);
   $rrc_exact_set("tb.omitted_recovery",32'h00200000,5,0,1);
   $rrc_exact_set("tb.omitted_removal",0,1,32'h00200000,7);
   $sdf_annotate("ivltests/recrem_hdl_precision.sdf",tb);
   $rrc_exact_expect("tb.recovery_only",0,17,32'h00200000,1);
   $rrc_exact_expect("tb.removal_only",32'h00200000,3,0,19);
   $rrc_exact_expect("tb.omitted_recovery",32'h00200000,5,0,23);
   $rrc_exact_expect("tb.omitted_removal",0,29,32'h00200000,7);
   $rrc_exact_finish;
   $finish;
 end
endmodule
