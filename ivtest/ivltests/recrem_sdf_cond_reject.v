// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module recrem_cond_reject(C,R,E,F,G);
 input C,R,E,F,G;
 reg notifier; initial notifier=0;
 specify
  $recrem(posedge R &&& (E),posedge C &&& (F === 1'b0),5,5,notifier);
 endspecify
endmodule
module tb;
 reg C,R,E,F;
 recrem_cond_reject u(C,R,E,F,E),valid(C,R,E,F,E);
reg quiet_clk;
recrem_cond_reject neg_first(quiet_clk,R,E,F,E),neg_second(quiet_clk,R,E,F,E);
initial begin quiet_clk=0; C=0;R=0;E=0;F=1'bx; #90 E=1;F=0; #10 R=1; #15 C=1; #1 R=0; #4 R=1; end
initial begin #1;
`ifdef VLOG95_SDF
// Tied input ports specialize the translated module names.
$sdf_annotate("ivltests/recrem_sdf_cond_reject_vlog95.sdf",tb);
`else
$sdf_annotate("ivltests/recrem_sdf_cond_reject.sdf",tb);
`endif
$rrc_population("tb.u",1); $rrc_population("tb.valid",1);
$rrc_watch("tb.u", "recrem", "R", "posedge", "C", "posedge", 5, 5, 5, 5, "notifier", 1'b0, 0);
$rrc_watch("tb.valid", "recrem", "R", "posedge", "C", "posedge", 20, 20, 8, 8, "notifier", 1'b0, 2, 115, 120);
$rrc_watch("tb.neg_first", "recrem", "R", "posedge", "C", "posedge", 0, 0, 99, 99, "notifier", 1'b0, 0);
$rrc_watch("tb.neg_second", "recrem", "R", "posedge", "C", "posedge", 99, 99, 0, 0, "notifier", 1'b0, 0);
$rrc_start; #159; $rrc_finish; $finish; end
endmodule
