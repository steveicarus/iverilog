// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module rrc_const(C,D);input C,D;reg notifier;initial notifier=0;
 specify specparam REC=4+6,REM=3*2;
 $recrem(posedge D,posedge C,REC,REM,notifier,,,,);
 endspecify endmodule
module tb;reg C,D;rrc_const u(C,D);
 initial begin C=0;D=0;#100 D=1;#9 C=1;end
initial begin #1;

$rrc_population("tb.u", 1);
$rrc_watch("tb.u", "recrem", "D", "posedge", "C", "posedge", 10, 10, 6, 6, "notifier", 1'b0, 1, 109);
$rrc_start; #139; $rrc_finish; $finish; end
endmodule
