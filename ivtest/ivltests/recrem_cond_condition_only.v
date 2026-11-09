// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module leaf(C,R,RE,CE);parameter REC=10,REM=6; input C,R,RE,CE;reg notifier;initial notifier=0;
 specify $recrem(posedge R &&& RE,posedge C &&& CE,REC,REM,notifier);endspecify
endmodule
module tb;
 reg C,R,RE,CE;leaf u(C,R,RE,CE);
 initial begin C=0;R=0;RE=1;CE=1;
 #100 R=1;#3 C=1;#1 RE=0;CE=0;#1 RE=1;CE=1;
 #5 C=0;#10 R=0;#80 R=1;#5 C=1;end
 initial begin #1;
$rrc_population("tb.u", 1);
$rrc_watch("tb.u", "recrem", "R", "posedge", "C", "posedge", 10, 10, 6, 6, "notifier", 2, 103, 205);
 $rrc_start; #249; $rrc_finish; $finish; end
endmodule
