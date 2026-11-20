// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module setuphold_cond_exact(C,D,E,F);
 input C,D,E,F;
 specify
  $setuphold(posedge C &&& (E),D &&& (F === 1'b0),1,1);
 endspecify
endmodule
module tb;
 reg C,D,E,F;
 setuphold_cond_exact first(C,D,E,F);
 setuphold_cond_exact second(C,D,E,F);
 setuphold_cond_exact omit_first(C,D,E,F);
 setuphold_cond_exact omit_second(C,D,E,F);
initial begin C=0;D=0;E=0;F=1'bx; #1;
$shc_exact_set("tb.first",0,1,32'h00200000,1);
$shc_exact_set("tb.second",32'h00200000,3,0,1);
$shc_exact_set("tb.omit_first",32'h00200000,5,0,1);
$shc_exact_set("tb.omit_second",0,1,32'h00200000,7);
$sdf_annotate("ivltests/setuphold_sdf_cond_exact.sdf",tb);
$shc_exact_expect("tb.first",0,17,32'h00200000,1);
$shc_exact_expect("tb.second",32'h00200000,3,0,19);
$shc_exact_expect("tb.omit_first",32'h00200000,5,0,23);
$shc_exact_expect("tb.omit_second",0,29,32'h00200000,7);
$shc_exact_finish; $finish; end
endmodule
