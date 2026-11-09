// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`include "ivltests/recrem_cells.vh"
module tb; reg C,D; rrc_pos #(0,6) hs(C,D);rrc_pos #(10,0) ss(C,D);rrc_pos #(0,0) zz(C,D);
 initial begin C=0;D=0;
 #100 D=1;#0 C=1;#20 C=0;D=0;
 #80 C=1;#0 D=1;#20 C=0;D=0;
 #80 D=1;#9 C=1;#11 C=0;D=0;
 #80 C=1;#5 D=1; end
initial begin #1;

$rrc_population("tb.hs", 1);
$rrc_population("tb.ss", 1);
$rrc_population("tb.zz", 1);
$rrc_watch("tb.hs", "recrem", "D", "posedge", "C", "posedge", 0, 0, 6, 6, "notifier", 1, 405);
$rrc_watch("tb.ss", "recrem", "D", "posedge", "C", "posedge", 10, 10, 0, 0, "notifier", 2, 100, 309);
$rrc_watch("tb.zz", "recrem", "D", "posedge", "C", "posedge", 0, 0, 0, 0, "notifier", 0);
$rrc_start; #449; $rrc_finish; $finish; end
endmodule
