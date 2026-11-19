// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ns/10ps
module rr_limits(R,C);
 parameter LIMIT=0;
 input R,C;reg nr,nm;initial begin nr=0;nm=0;end
 specify specparam SCALE=1;
 $recovery(posedge R,posedge C,LIMIT*SCALE,nr);
 $removal(posedge R,posedge C,LIMIT*SCALE,nm);
 endspecify
endmodule
`timescale 1ps/1ps
module tb;
 reg R,C;rr_limits #(0) zero(R,C);rr_limits #(0.004) rounded(R,C);rr_limits #(0.016) positive(R,C);
 initial begin R=0;C=0;#100 R=1;#19 C=1;#1 R=0;#10 R=1;end
 initial begin #1;
    $rr_watch("tb.zero", "recovery", "R", "posedge", "C", "posedge", 0.0, 0, "nr", 1'b0, 0);
    $rr_watch("tb.zero", "removal", "R", "posedge", "C", "posedge", 0.0, 0, "nm", 1'b0, 0);
    $rr_watch("tb.rounded", "recovery", "R", "posedge", "C", "posedge", 0.0, 0, "nr", 1'b0, 0);
    $rr_watch("tb.rounded", "removal", "R", "posedge", "C", "posedge", 0.0, 0, "nm", 1'b0, 0);
    $rr_watch("tb.positive", "recovery", "R", "posedge", "C", "posedge", 0.02, 20, "nr", 1'b0, 1, 119);
    $rr_watch("tb.positive", "removal", "R", "posedge", "C", "posedge", 0.02, 20, "nm", 1'b0, 1, 130);
 $rr_start;#149;$rr_finish;$finish;
 end
endmodule
