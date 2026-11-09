// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module alias_leaf(A,RE,CE);parameter REC=10,REM=6;input A,RE,CE;reg notifier;initial notifier=0;
 specify $recrem(posedge A &&& RE,posedge A &&& CE,REC,REM,notifier);endspecify endmodule
module notifier_leaf(A);input A;reg notifier;initial notifier=0;
 specify $recrem(posedge A &&& (notifier === 1'b0),posedge A &&& (notifier === 1'b0),10,6,notifier);endspecify endmodule
module tb;
 reg A,RE,CE;alias_leaf p(A,RE,CE);alias_leaf #(10,0) rz(A,RE,CE);alias_leaf #(0,6) mz(A,RE,CE);alias_leaf #(0,0) zz(A,RE,CE);notifier_leaf nn(A);
 initial begin A=0;RE=1;CE=1;#100 A=1;#20 A=0;#70 CE=0;#10 A=1;#20 A=0;
 #70 RE=0;CE=1;#10 A=1;#20 A=0;#70 RE=1;#10 A=1;end
 initial begin #1;
$rrc_population("tb.p", 1);
$rrc_watch("tb.p", "recrem", "A", "posedge", "A", "posedge", 10, 10, 6, 6, "notifier", 1'b0, 2, 100, 400);
$rrc_population("tb.rz", 1);
$rrc_watch("tb.rz", "recrem", "A", "posedge", "A", "posedge", 10, 10, 0, 0, "notifier", 1'b0, 2, 100, 400);
$rrc_population("tb.mz", 1);
$rrc_watch("tb.mz", "recrem", "A", "posedge", "A", "posedge", 0, 0, 6, 6, "notifier", 1'b0, 0);
$rrc_population("tb.zz", 1);
$rrc_watch("tb.zz", "recrem", "A", "posedge", "A", "posedge", 0, 0, 0, 0, "notifier", 1'b0, 0);
$rrc_population("tb.nn", 1);
$rrc_watch("tb.nn", "recrem", "A", "posedge", "A", "posedge", 10, 10, 6, 6, "notifier", 1'b0, 1, 100);
 $rrc_start; #449; $rrc_finish; $finish; end
endmodule
