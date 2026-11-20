// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module recrem_cond_exact(C,R,E,F);
 input C,R,E,F;
 specify
  $recrem(posedge R &&& (E),posedge C &&& (F === 1'b0),1,1);
 endspecify
endmodule
module tb;
 reg C,R,E,F;
 recrem_cond_exact first(C,R,E,F);
 recrem_cond_exact second(C,R,E,F);
 recrem_cond_exact omit_first(C,R,E,F);
 recrem_cond_exact omit_second(C,R,E,F);
initial begin C=0;R=0;E=0;F=1'bx; #1;
$rrc_exact_set("tb.first",0,1,32'h00200000,1);
$rrc_exact_set("tb.second",32'h00200000,3,0,1);
$rrc_exact_set("tb.omit_first",32'h00200000,5,0,1);
$rrc_exact_set("tb.omit_second",0,1,32'h00200000,7);
$sdf_annotate("ivltests/recrem_sdf_cond_exact.sdf",tb);
$rrc_exact_expect("tb.first",0,17,32'h00200000,1);
$rrc_exact_expect("tb.second",32'h00200000,3,0,19);
$rrc_exact_expect("tb.omit_first",32'h00200000,5,0,23);
$rrc_exact_expect("tb.omit_second",0,29,32'h00200000,7);
$rrc_exact_finish; $finish; end
endmodule
