// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module shc_const(C,D);input C,D;reg notifier;initial notifier=0;
 specify specparam S=4+6,H=3*2;
 $setuphold(posedge C,posedge D,S,H,notifier,,,,);
 endspecify endmodule
module tb;reg C,D;shc_const u(C,D);
 initial begin C=0;D=0;#100 D=1;#9 C=1;end
initial begin #1;

$shc_population("tb.u", 1);
$shc_watch("tb.u", "setuphold", "C", "posedge", "D", "posedge", 10, 10, 6, 6, "notifier", 1'b0, 1, 109);
$shc_start; #139; $shc_finish; $finish; end
endmodule
