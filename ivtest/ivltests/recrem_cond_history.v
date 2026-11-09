// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module leaf(C,R,RE,CE);parameter REC=10,REM=6; input C,R,RE,CE;reg notifier;initial notifier=0;
 specify $recrem(posedge R &&& RE,posedge C &&& CE,REC,REM,notifier);endspecify
endmodule
module tb;
 reg C0,R0,E0,C1,R1,E1,C2,R2,E2,C3,R3,E3;
 leaf keep_r(C0,R0,E0,1'b1),stale_r(C1,R1,E1,1'b1);
 leaf keep_c(C2,R2,1'b1,E2),stale_c(C3,R3,1'b1,E3);
 initial begin C0=0;R0=0;E0=1;#100 R0=1;#2 R0=0;#1 E0=0;#1 R0=1;#3 E0=1;#1 C0=1;end
 initial begin C1=0;R1=0;E1=1;#100 R1=1;#2 R1=0;#5 E1=0;#1 R1=1;#2 E1=1;#1 C1=1;end
 initial begin C2=0;R2=0;E2=1;#100 C2=1;#2 C2=0;#1 E2=0;#1 C2=1;#1 R2=1;end
 initial begin C3=0;R3=0;E3=1;#100 C3=1;#2 C3=0;#5 E3=0;#1 C3=1;#2 E3=1;#1 R3=1;end
 initial begin #1;
$rrc_population("tb.keep_r", 1);
$rrc_watch("tb.keep_r", "recrem", "R", "posedge", "C", "posedge", 10, 10, 6, 6, "notifier", 1, 108);
$rrc_population("tb.stale_r", 1);
$rrc_watch("tb.stale_r", "recrem", "R", "posedge", "C", "posedge", 10, 10, 6, 6, "notifier", 0);
$rrc_population("tb.keep_c", 1);
$rrc_watch("tb.keep_c", "recrem", "R", "posedge", "C", "posedge", 10, 10, 6, 6, "notifier", 1, 105);
$rrc_population("tb.stale_c", 1);
$rrc_watch("tb.stale_c", "recrem", "R", "posedge", "C", "posedge", 10, 10, 6, 6, "notifier", 0);
 $rrc_start; #179; $rrc_finish; $finish; end
endmodule
