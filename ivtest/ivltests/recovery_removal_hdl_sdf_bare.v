// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module rr_bare(R,C);
 input R,C;reg np,nn,nm;initial begin np=0;nn=0;nm=0;end
 specify $recovery(posedge R,posedge C,5,np);$recovery(negedge R,negedge C,5,nn);$removal(posedge R,posedge C,7,nm);endspecify
endmodule
module tb;
 reg R,C;rr_bare u(R,C);
 initial begin R=0;C=0;#100 R=1;#9 C=1;#1 R=0;#9 C=0;end
 initial begin #1;
 $sdf_annotate("ivltests/recovery_removal_hdl_bare.sdf",tb);
 $rr_watch("tb.u", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "np", 1'b0, 1, 109);
 // Initial X-to-0 R then C violates the HDL limit 5 before SDF is applied.
 $rr_watch("tb.u", "recovery", "R", "negedge", "C", "negedge", 10.0, 10, "nn", 1'b0, 2, 0, 119);
 $rr_watch("tb.u", "removal", "R", "posedge", "C", "posedge", 7.0, 7, "nm", 1'b0, 0);
 $rr_start;#139;$rr_finish;$finish;
 end
endmodule
