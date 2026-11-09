// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module setuphold_cond_escaped(\C.0 ,\D/0 ,\E/0 ,\F\0 );
 input \C.0 ,\D/0 ,\E/0 ,\F\0 ;
 reg notifier; initial notifier=0;
 specify
  $setuphold(posedge \C.0  &&& (\E/0  === 1'b0),\D/0  &&& (\F\0 ),5,5,notifier);
 endspecify
endmodule
module tb;
 reg C,D,E,F;
 setuphold_cond_escaped u(C,D,E,F);
initial begin C=0;D=0;E=1'bx;F=0; #90 E=0;F=1; #10 D=1; #15 C=1; #5 D=0; end
initial begin #1; $sdf_annotate("ivltests/setuphold_sdf_cond_escaped.sdf",tb);
$shc_population("tb.u",1);
$shc_watch("tb.u", "setuphold", "C.0", "posedge", "D/0", "any", 20, 20, 8, 8, "notifier", 1'b0, 2, 115, 120);
$tchk_condition("tb.u", "notifier", "reference", "case", "E/0", 0);
$tchk_condition("tb.u", "notifier", "data", "bare", "F\\0", 0);
$shc_start; #90;
$tchk_condition("tb.u", "notifier", "reference", "case", "E/0", 0);
$tchk_condition("tb.u", "notifier", "data", "bare", "F\\0", 0);
#69; $shc_finish; $finish; end
endmodule
