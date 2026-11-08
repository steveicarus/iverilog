// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module rr_sdf_leaf(R,C,E,K);
 input R,C,E,K;reg nr,nm,ne,nk,nn;initial begin nr=0;nm=0;ne=0;nk=0;nn=0;end
 specify
 $recovery(posedge R,posedge C,5,nr);
 $removal(posedge R,posedge C,5,nm);
 $recovery(posedge E,posedge C,9,ne);
 $recovery(posedge R,posedge K,9,nk);
 $recovery(posedge R,negedge C,5,nn);
 endspecify
endmodule
`timescale 1ps/1ps
module tb;
 reg R,C,E,K;rr_sdf_leaf u(R,C,E,K);
 initial begin R=0;C=0;E=0;K=0;
 #100 R=1;#15 C=1;#1 R=0;#4 R=1;#10 C=0;#1 R=0;
 #9 R=1;#20 C=1;#1 R=0;#7 R=1;#12 C=0;
 #10 E=1;#5 C=1;#14 R=0;#1 R=1;#5 C=0;end
 initial begin #114 K=1;#2 K=0;#27 K=1;end
 initial begin #1;
 $sdf_annotate("ivltests/recovery_removal_hdl.sdf",tb);
`ifdef RR_MIN
    $rr_watch("tb.u", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "nr", 0);
    $rr_watch("tb.u", "removal", "R", "posedge", "C", "posedge", 3.0, 3, "nm", 0);
`elsif RR_MAX
    $rr_watch("tb.u", "recovery", "R", "posedge", "C", "posedge", 30.0, 30, "nr", 3, 115, 160, 195);
    $rr_watch("tb.u", "removal", "R", "posedge", "C", "posedge", 13.0, 13, "nm", 2, 120, 168);
`else
    $rr_watch("tb.u", "recovery", "R", "posedge", "C", "posedge", 20.0, 20, "nr", 1, 115);
    $rr_watch("tb.u", "removal", "R", "posedge", "C", "posedge", 8.0, 8, "nm", 1, 120);
`endif
    $rr_watch("tb.u", "recovery", "E", "posedge", "C", "posedge", 9.0, 9, "ne", 1, 195);
    $rr_watch("tb.u", "recovery", "R", "posedge", "K", "posedge", 9.0, 9, "nk", 1, 143);
    $rr_watch("tb.u", "recovery", "R", "posedge", "C", "negedge", 5.0, 5, "nn", 0);
 $rr_start;#229;$rr_finish;$finish;
 end
endmodule
