// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module leaf(C,R,RE,CE);parameter REC=10,REM=6; input C,R,RE,CE;reg notifier;initial notifier=0;
 specify $recrem(posedge R,posedge C &&& CE,REC,REM,notifier);endspecify
endmodule
module tb;
 reg C,R,CE;leaf u(C,R,1'b1,CE);
 initial begin C=0;R=0;CE=0;
 #100 R=1;#5 C=1;#15 C=0;R=0;#70 CE=1;#10 C=1;#5 R=1;
 #1 CE=0;#2 C=0;#2 C=1;#1 R=0;#1 R=1;end
 initial begin #1;
$rrc_population("tb.u", 1);
$rrc_watch("tb.u", "recrem", "R", "posedge", "C", "posedge", 10, 10, 6, 6, "notifier", 1'b0, 1, 205);
 $rrc_start; #249; $rrc_finish; $finish; end
endmodule
