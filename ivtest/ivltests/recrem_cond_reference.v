// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module leaf(C,R,RE,CE);parameter REC=10,REM=6; input C,R,RE,CE;reg notifier;initial notifier=0;
 specify $recrem(posedge R &&& RE,posedge C,REC,REM,notifier);endspecify
endmodule
module tb;
 reg C,R,RE;leaf u(C,R,RE,1'b1);
 initial begin C=0;R=0;RE=0;
 #100 R=1;#5 C=1;#15 C=0;R=0;#70 RE=1;#10 R=1;#5 C=1;
 #1 RE=0;#2 R=0;#2 R=1;#1 C=0;#1 C=1;end
 initial begin #1;
$rrc_population("tb.u", 1);
$rrc_watch("tb.u", "recrem", "R", "posedge", "C", "posedge", 10, 10, 6, 6, "notifier", 1, 205);
 $rrc_start; #249; $rrc_finish; $finish; end
endmodule
