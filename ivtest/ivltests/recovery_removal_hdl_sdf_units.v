// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ns/10ps
module rr_escaped(dummy,\R.dot ,\C\tail );
 input dummy,\R.dot ,\C\tail ;reg nr,nm;initial begin nr=0;nm=0;end
 specify $recovery(posedge \R.dot ,posedge \C\tail ,0.005,nr);
 $removal(posedge \R.dot ,posedge \C\tail ,0.005,nm);endspecify
endmodule
`timescale 1ps/1ps
module tb;
 reg R,C;rr_escaped u(1'b0,R,C);
 initial begin R=0;C=0;#100 R=1;#30 C=1;#1 R=0;#14 R=1;end
 initial begin #1;
 $sdf_annotate("ivltests/recovery_removal_hdl_units.sdf",tb);
    $rr_watch("tb.u", "recovery", "R.dot", "posedge", "C\\tail", "posedge", 0.04, 40, "nr", 1, 130);
    $rr_watch("tb.u", "removal", "R.dot", "posedge", "C\\tail", "posedge", 0.02, 20, "nm", 1, 145);
 $rr_start;#169;$rr_finish;$finish;
 end
endmodule
