// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module leaf(C,R,RE,CE);parameter REC=10,REM=6; input C,R,RE,CE;reg notifier;initial notifier=0;
 specify $recrem(posedge R &&& RE,posedge C &&& CE,REC,REM,notifier);endspecify
endmodule
module tb;
 reg C,R,RE,CE;leaf p(C,R,RE,CE);leaf #(10,0) rz(C,R,RE,CE);leaf #(0,6) mz(C,R,RE,CE);leaf #(0,0) zz(C,R,RE,CE);
 initial begin C=0;R=0;RE=1;CE=1;
 #100 R=1;#0 C=1;#20 R=0;C=0;#180 C=1;#0 R=1;#20 R=0;C=0;
 #170 CE=0;#10 R=1;#0 C=1;#20 R=0;C=0;#170 RE=0;CE=1;#10 R=1;#0 C=1;end
 initial begin #1;
$rrc_population("tb.p", 1);
$rrc_watch("tb.p", "recrem", "R", "posedge", "C", "posedge", 10, 10, 6, 6, "notifier", 1'b0, 2, 100, 300);
$rrc_population("tb.rz", 1);
$rrc_watch("tb.rz", "recrem", "R", "posedge", "C", "posedge", 10, 10, 0, 0, "notifier", 1'b0, 1, 100);
$rrc_population("tb.mz", 1);
$rrc_watch("tb.mz", "recrem", "R", "posedge", "C", "posedge", 0, 0, 6, 6, "notifier", 1'b0, 0);
$rrc_population("tb.zz", 1);
$rrc_watch("tb.zz", "recrem", "R", "posedge", "C", "posedge", 0, 0, 0, 0, "notifier", 1'b0, 0);
 $rrc_start; #749; $rrc_finish; $finish; end
endmodule
