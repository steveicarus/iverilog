// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ns/10ps
module rrc_units(C,D);parameter REC=0.016,REM=0.026;input C,D;reg notifier;initial notifier=0;
 specify $recrem(D,posedge C,REC,REM,notifier);endspecify endmodule
`timescale 1ps/1ps
module tb;reg C,D;rrc_units p(C,D);rrc_units #(0.004,0.004) z(C,D);
 initial begin C=0;D=0;#100 D=1;#19 C=1;#29 D=0;#2 D=1;end
initial begin #1;

$rrc_population("tb.p", 1);
$rrc_population("tb.z", 1);
$rrc_watch("tb.p", "recrem", "D", "any", "C", "posedge", 0.02, 20, 0.03, 30, "notifier", 2, 119, 148);
$rrc_watch("tb.z", "recrem", "D", "any", "C", "posedge", 0, 0, 0, 0, "notifier", 0);
$rrc_start; #199; $rrc_finish; $finish; end
endmodule
