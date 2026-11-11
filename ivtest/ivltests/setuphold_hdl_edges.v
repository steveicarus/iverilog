// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`include "ivltests/setuphold_cells.vh"
module shc_neg(C,D);input C,D;reg notifier;initial notifier=0;
 specify $setuphold(negedge C,posedge D,10,6,notifier);endspecify endmodule
module tb;reg C,D,NC,ND;shc_any u(C,D);shc_neg n(NC,ND);
 initial begin C=0;D=0;#99 D=1;#1 C=1'bx;#1 C=1;#1 D=1'bz;#1 D=1'bx;#1 D=0;#1 C=0;#45 D=1;#10 C=1;#6 D=0;end
 initial begin NC=1;ND=0;#100 ND=1;#9 NC=1'bx;#1 NC=0;#5 ND=0;#15 NC=1;#70 ND=1;#10 NC=0;#2 ND=0;#4 ND=1;end
initial begin #1;

$shc_population("tb.u", 1);
$shc_population("tb.n", 1);
$shc_watch("tb.u", "setuphold", "C", "posedge", "D", "any", 10, 10, 6, 6, "notifier", 1'b0, 5, 100, 101, 102, 103, 104);
$shc_watch("tb.n", "setuphold", "C", "negedge", "D", "posedge", 10, 10, 6, 6, "notifier", 1'b0, 1, 109);
$shc_start; #249; $shc_finish; $finish; end
endmodule
