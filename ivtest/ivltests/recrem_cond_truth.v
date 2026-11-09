// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module ref_b(C,R,RE,CE);parameter REC=10,REM=6; input C,R,RE,CE;reg notifier;initial notifier=0;
 specify $recrem(posedge R &&& RE,posedge C,REC,REM,notifier);endspecify
endmodule
module ref_0(C,R,RE,CE);parameter REC=10,REM=6; input C,R,RE,CE;reg notifier;initial notifier=0;
 specify $recrem(posedge R &&& (RE === 1'b0),posedge C,REC,REM,notifier);endspecify
endmodule
module ref_1(C,R,RE,CE);parameter REC=10,REM=6; input C,R,RE,CE;reg notifier;initial notifier=0;
 specify $recrem(posedge R &&& (RE === 1'b1),posedge C,REC,REM,notifier);endspecify
endmodule
module data_b(C,R,RE,CE);parameter REC=10,REM=6; input C,R,RE,CE;reg notifier;initial notifier=0;
 specify $recrem(posedge R,posedge C &&& CE,REC,REM,notifier);endspecify
endmodule
module data_0(C,R,RE,CE);parameter REC=10,REM=6; input C,R,RE,CE;reg notifier;initial notifier=0;
 specify $recrem(posedge R,posedge C &&& (CE === 1'b0),REC,REM,notifier);endspecify
endmodule
module data_1(C,R,RE,CE);parameter REC=10,REM=6; input C,R,RE,CE;reg notifier;initial notifier=0;
 specify $recrem(posedge R,posedge C &&& (CE === 1'b1),REC,REM,notifier);endspecify
endmodule
module tb;
 reg C,R,E;ref_b rb(C,R,E,E);ref_0 r0(C,R,E,E);ref_1 r1(C,R,E,E);data_b cb(C,R,E,E);data_0 c0(C,R,E,E);data_1 c1(C,R,E,E);
 initial begin C=0;R=0;E=0;
 #100 R=1;#5 C=1;#15 R=0;C=0;#70 E=1;#110 R=1;#5 C=1;#15 R=0;C=0;
 #70 E=1'bx;#110 R=1;#5 C=1;#15 R=0;C=0;#70 E=1'bz;#110 R=1;#5 C=1;end
 initial begin #1;
$rrc_population("tb.rb", 1);
$rrc_watch("tb.rb", "recrem", "R", "posedge", "C", "posedge", 10, 10, 6, 6, "notifier", 1, 305);
$rrc_population("tb.r0", 1);
$rrc_watch("tb.r0", "recrem", "R", "posedge", "C", "posedge", 10, 10, 6, 6, "notifier", 1, 105);
$rrc_population("tb.r1", 1);
$rrc_watch("tb.r1", "recrem", "R", "posedge", "C", "posedge", 10, 10, 6, 6, "notifier", 1, 305);
$rrc_population("tb.cb", 1);
$rrc_watch("tb.cb", "recrem", "R", "posedge", "C", "posedge", 10, 10, 6, 6, "notifier", 1, 305);
$rrc_population("tb.c0", 1);
$rrc_watch("tb.c0", "recrem", "R", "posedge", "C", "posedge", 10, 10, 6, 6, "notifier", 1, 105);
$rrc_population("tb.c1", 1);
$rrc_watch("tb.c1", "recrem", "R", "posedge", "C", "posedge", 10, 10, 6, 6, "notifier", 1, 305);
 $rrc_start; #749; $rrc_finish; $finish; end
endmodule
