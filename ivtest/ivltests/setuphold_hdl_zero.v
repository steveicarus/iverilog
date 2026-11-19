// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`include "ivltests/setuphold_cells.vh"
module tb; reg C,D; shc_pos #(0,6) hs(C,D);shc_pos #(10,0) ss(C,D);shc_pos #(0,0) zz(C,D);
 initial begin C=0;D=0;
 #100 D=1;#0 C=1;#20 C=0;D=0;
 #80 C=1;#0 D=1;#20 C=0;D=0;
 #80 D=1;#9 C=1;#11 C=0;D=0;
 #80 C=1;#5 D=1; end
initial begin #1;

$shc_population("tb.hs", 1);
$shc_population("tb.ss", 1);
$shc_population("tb.zz", 1);
$shc_watch("tb.hs", "setuphold", "C", "posedge", "D", "posedge", 0, 0, 6, 6, "notifier", 1'b0, 2, 200, 405);
$shc_watch("tb.ss", "setuphold", "C", "posedge", "D", "posedge", 10, 10, 0, 0, "notifier", 1'b0, 1, 309);
$shc_watch("tb.zz", "setuphold", "C", "posedge", "D", "posedge", 0, 0, 0, 0, "notifier", 1'b0, 0);
$shc_start; #449; $shc_finish; $finish; end
endmodule
