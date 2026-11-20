// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module leaf(C,R,RE,CE);input C,R,RE,CE;reg notifier;initial notifier=0;
 specify $recrem(posedge R &&& RE,posedge C &&& CE,10,6,notifier);endspecify
endmodule
module tb;
 reg C,R,RE,CE;leaf u(C,R,RE,CE);
 initial begin C=0;R=0;RE=0;CE=0;#1 $sdf_annotate("ivltests/recrem_cond_reject.sdf",u);
 #49 RE=1;CE=1;#50 R=1;#15 C=1;#1 R=0;#4 R=1;end
 initial begin #2;
$rrc_population("tb.u", 1);
$rrc_watch("tb.u", "recrem", "R", "posedge", "C", "posedge", 10, 10, 6, 6, "notifier", 1'b0, 1, 120);
 $rrc_start; #158; $rrc_finish; $finish; end
endmodule
