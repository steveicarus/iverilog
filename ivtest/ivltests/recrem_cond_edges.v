// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module negative_leaf(C,R,RE,CE);parameter REC=10,REM=6; input C,R,RE,CE;reg notifier;initial notifier=0;
 specify $recrem(negedge R &&& RE,negedge C &&& CE,REC,REM,notifier);endspecify
endmodule
module any_leaf(C,R,RE,CE);parameter REC=10,REM=6; input C,R,RE,CE;reg notifier;initial notifier=0;
 specify $recrem(R &&& RE,posedge C &&& CE,REC,REM,notifier);endspecify
endmodule
module tb;
 reg CN,RN,ERN,ECN,CA,RA,EA,CM,RM,EM;
 negative_leaf n(CN,RN,ERN,ECN),nmask(CM,RM,EM,1'b1);any_leaf a(CA,RA,EA,EA);
 initial begin CN=1;RN=1;ERN=1;ECN=1;#100 RN=0;#5 CN=0;#15 ERN=0;#10 RN=1;CN=1;
 #70 RN=0;#5 CN=0;#15 RN=1;CN=1;#70 ERN=1;ECN=0;#10 CN=0;#5 RN=0;
 #15 CN=1;RN=1;#70 ECN=1;#10 CN=0;#5 RN=0;end
 initial begin CM=1;RM=1;EM=0;#100 RM=0;#1 EM=1;#1 RM=1'bx;#3 CM=0;
 #5 CM=1;#10 RM=0;#5 CM=0;end
 initial begin CA=0;RA=0;EA=0;#10 EA=1;#90 RA=1;#5 CA=1;#3 RA=0;
 #12 CA=0;RA=1;#5 CA=1;#1 EA=0;#2 RA=0;#1 EA=1;#1 CA=0;#5 CA=1;end
 initial begin #1;
$rrc_population("tb.n", 1);
$rrc_watch("tb.n", "recrem", "R", "negedge", "C", "negedge", 10, 10, 6, 6, "notifier", 2, 105, 405);
$rrc_population("tb.a", 1);
$rrc_watch("tb.a", "recrem", "R", "any", "C", "posedge", 10, 10, 6, 6, "notifier", 3, 105, 108, 125);
$rrc_population("tb.nmask", 1);
$rrc_watch("tb.nmask", "recrem", "R", "negedge", "C", "negedge", 10, 10, 6, 6, "notifier", 1, 125);
 $rrc_start; #449; $rrc_finish; $finish; end
endmodule
