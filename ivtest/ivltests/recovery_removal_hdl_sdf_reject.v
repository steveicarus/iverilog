// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module rr_reject(R,C,OTHER);
 input R,C,OTHER;reg nr,nm;initial begin nr=0;nm=0;end
 specify $recovery(posedge R,posedge C,10,nr);$removal(posedge R,posedge C,10,nm);endspecify
endmodule
module tb;
 reg R,C,OTHER;
 rr_reject wrong_control(R,C,OTHER);
 rr_reject wrong_clock(R,C,OTHER);
 rr_reject wrong_edge(R,C,OTHER);
 rr_reject negative(R,C,OTHER);
 rr_reject overflow(R,C,OTHER);
 rr_reject wrong_cell(R,C,OTHER);
 rr_reject conditional(R,C,OTHER);
 rr_reject hierarchy(R,C,OTHER);
 rr_reject valid(R,C,OTHER);
 rr_reject combined(R,C,OTHER);
 initial begin R=0;C=0;OTHER=0;end
 initial begin #1;
 $sdf_annotate("ivltests/recovery_removal_hdl_reject.sdf",tb);
 $rr_watch("tb.wrong_control", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "nr", 1'b0, 0);
 $rr_watch("tb.wrong_control", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "nm", 1'b0, 0);
 $rr_watch("tb.wrong_clock", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "nr", 1'b0, 0);
 $rr_watch("tb.wrong_clock", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "nm", 1'b0, 0);
 $rr_watch("tb.wrong_edge", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "nr", 1'b0, 0);
 $rr_watch("tb.wrong_edge", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "nm", 1'b0, 0);
 $rr_watch("tb.negative", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "nr", 1'b0, 0);
 $rr_watch("tb.negative", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "nm", 1'b0, 0);
 $rr_watch("tb.overflow", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "nr", 1'b0, 0);
 $rr_watch("tb.overflow", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "nm", 1'b0, 0);
 $rr_watch("tb.wrong_cell", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "nr", 1'b0, 0);
 $rr_watch("tb.wrong_cell", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "nm", 1'b0, 0);
 $rr_watch("tb.conditional", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "nr", 1'b0, 0);
 $rr_watch("tb.conditional", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "nm", 1'b0, 0);
 $rr_watch("tb.hierarchy", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "nr", 1'b0, 0);
 $rr_watch("tb.hierarchy", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "nm", 1'b0, 0);
 $rr_watch("tb.valid", "recovery", "R", "posedge", "C", "posedge", 30.0, 30, "nr", 1'b0, 0);
 $rr_watch("tb.valid", "removal", "R", "posedge", "C", "posedge", 30.0, 30, "nm", 1'b0, 0);
 $rr_watch("tb.combined", "recovery", "R", "posedge", "C", "posedge", 30.0, 30, "nr", 1'b0, 0);
 $rr_watch("tb.combined", "removal", "R", "posedge", "C", "posedge", 30.0, 30, "nm", 1'b0, 0);
 $rr_start;#9;$rr_finish;
 end
endmodule
