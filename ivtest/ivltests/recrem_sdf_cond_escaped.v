// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module recrem_cond_escaped(\R/0 ,\C.0 ,\E/0 ,\F\0 );
 input \R/0 ,\C.0 ,\E/0 ,\F\0 ;
 reg notifier; initial notifier=0;
 specify
  $recrem(posedge \R/0  &&& (\E/0  === 1'b0),posedge \C.0  &&& (\F\0 ),5,5,notifier);
 endspecify
endmodule
module tb;
 reg C,R,E,F;
 recrem_cond_escaped u(R,C,E,F);
initial begin C=0;R=0;E=1'bx;F=0; #90 E=0;F=1; #10 R=1; #15 C=1; #1 R=0; #4 R=1; end
initial begin #1; $sdf_annotate("ivltests/recrem_sdf_cond_escaped.sdf",tb);
$rrc_population("tb.u",1);
$rrc_watch("tb.u", "recrem", "R/0", "posedge", "C.0", "posedge", 20, 20, 8, 8, "notifier", 1'b0, 2, 115, 120);
$tchk_condition("tb.u", "notifier", "reference", "case", "E/0", 0);
$tchk_condition("tb.u", "notifier", "data", "bare", "F\\0", 0);
$rrc_start; #90;
$tchk_condition("tb.u", "notifier", "reference", "case", "E/0", 0);
$tchk_condition("tb.u", "notifier", "data", "bare", "F\\0", 0);
#69; $rrc_finish; $finish; end
endmodule
