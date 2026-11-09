// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`include "ivltests/recrem_cells.vh"
module rrc_alias(A);parameter REC=10,REM=6;input A;reg notifier;initial notifier=0;
 specify $recrem(posedge A,posedge A,REC,REM,notifier);endspecify endmodule
module tb;reg A;rrc_pos twoports(A,A);rrc_alias p(A);rrc_alias #(0,6) h(A);rrc_alias #(10,0) s(A);rrc_alias #(0,0) z(A);
 initial begin A=0;#100 A=1;#20 A=0;#80 A=1;end
initial begin #1;

$rrc_population("tb.twoports", 1);
$rrc_population("tb.p", 1);
$rrc_population("tb.h", 1);
$rrc_population("tb.s", 1);
$rrc_population("tb.z", 1);
$rrc_watch("tb.p", "recrem", "A", "posedge", "A", "posedge", 10, 10, 6, 6, "notifier", 1'b0, 2, 100, 200);
$rrc_watch("tb.h", "recrem", "A", "posedge", "A", "posedge", 0, 0, 6, 6, "notifier", 1'b0, 0);
$rrc_watch("tb.s", "recrem", "A", "posedge", "A", "posedge", 10, 10, 0, 0, "notifier", 1'b0, 2, 100, 200);
$rrc_watch("tb.z", "recrem", "A", "posedge", "A", "posedge", 0, 0, 0, 0, "notifier", 1'b0, 0);
$rrc_watch("tb.twoports", "recrem", "D", "posedge", "C", "posedge", 10, 10, 6, 6, "notifier", 1'b0, 2, 100, 200);
$rrc_start; #249; $rrc_finish; $finish; end
endmodule
