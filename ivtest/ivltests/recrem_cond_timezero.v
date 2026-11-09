// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module ref_leaf(C,R,RE,CE);parameter REC=10,REM=6; input C,R,RE,CE;reg notifier;initial notifier=0;
 specify $recrem(posedge R &&& RE,posedge C,REC,REM,notifier);endspecify
endmodule
module clock_leaf(C,R,RE,CE);parameter REC=10,REM=6; input C,R,RE,CE;reg notifier;initial notifier=0;
 specify $recrem(posedge R,posedge C &&& CE,REC,REM,notifier);endspecify
endmodule
module tb;
 reg SC,SD,HC,HD,E;ref_leaf #(10,10) vr(SC,SD,1'b1,1'b1),mr(SC,SD,E,1'b1);clock_leaf #(10,10) vc(HC,HD,1'b1,1'b1),mc(HC,HD,1'b1,E);
 initial begin E=0;SC=0;SD=1;HC=1;HD=0;
 #9 SC=1;HD=1;#11 SD=0;HC=0;E=1;#1 SD=1;HC=1;#1 SC=0;HD=0;#3 SC=1;HD=1;end
 initial begin #1;
$rrc_population("tb.vr", 1);
$rrc_watch("tb.vr", "recrem", "R", "posedge", "C", "posedge", 10, 10, 10, 10, "notifier", 1'b0, 2, 9, 25);
$rrc_population("tb.mr", 1);
$rrc_watch("tb.mr", "recrem", "R", "posedge", "C", "posedge", 10, 10, 10, 10, "notifier", 1'b0, 1, 25);
$rrc_population("tb.vc", 1);
$rrc_watch("tb.vc", "recrem", "R", "posedge", "C", "posedge", 10, 10, 10, 10, "notifier", 1'b0, 2, 9, 25);
$rrc_population("tb.mc", 1);
$rrc_watch("tb.mc", "recrem", "R", "posedge", "C", "posedge", 10, 10, 10, 10, "notifier", 1'b0, 1, 25);
 $rrc_start; #59; $rrc_finish; $finish; end
endmodule
