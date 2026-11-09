// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`include "ivltests/recrem_cells.vh"
module rrc_neg(C,D);input C,D;reg notifier;initial notifier=0;
 specify $recrem(posedge D,negedge C,10,6,notifier);endspecify endmodule
module tb;reg C,D,NC,ND;rrc_any u(C,D);rrc_neg n(NC,ND);
 initial begin C=0;D=0;#99 D=1;#1 C=1'bx;#1 C=1;#1 D=1'bz;#1 D=1'bx;#1 D=0;#1 C=0;#45 D=1;#10 C=1;#6 D=0;end
 initial begin NC=1;ND=0;#100 ND=1;#9 NC=1'bx;#1 NC=0;#5 ND=0;#15 NC=1;#70 ND=1;#10 NC=0;#2 ND=0;#4 ND=1;end
initial begin #1;

$rrc_population("tb.u", 1);
$rrc_population("tb.n", 1);
$rrc_watch("tb.u", "recrem", "D", "any", "C", "posedge", 10, 10, 6, 6, "notifier", 5, 100, 101, 102, 103, 104);
$rrc_watch("tb.n", "recrem", "D", "posedge", "C", "negedge", 10, 10, 6, 6, "notifier", 1, 109);
$rrc_start; #249; $rrc_finish; $finish; end
endmodule
