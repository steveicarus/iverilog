// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module setuphold_cond_reject(C,D,E,F,G);
 input C,D,E,F,G;
 reg notifier; initial notifier=0;
 specify
  $setuphold(posedge C &&& (E),D &&& (F === 1'b0),5,5,notifier);
 endspecify
endmodule
module tb;
 reg C,D,E,F;
 setuphold_cond_reject u(C,D,E,F,E),valid(C,D,E,F,E);
initial begin C=0;D=0;E=0;F=1'bx; #90 E=1;F=0; #10 D=1; #15 C=1; #5 D=0; end
initial begin #1;
`ifdef VLOG95_SDF
// Tied input ports specialize the translated module names.
$sdf_annotate("ivltests/setuphold_sdf_cond_reject_vlog95.sdf",tb);
`else
$sdf_annotate("ivltests/setuphold_sdf_cond_reject.sdf",tb);
`endif
$shc_population("tb.u",1); $shc_population("tb.valid",1);
$shc_watch("tb.u", "setuphold", "C", "posedge", "D", "any", 5, 5, 5, 5, "notifier", 1'b0, 0);
$shc_watch("tb.valid", "setuphold", "C", "posedge", "D", "any", 20, 20, 8, 8, "notifier", 1'b0, 2, 115, 120);
$shc_start; #159; $shc_finish; $finish; end
endmodule
