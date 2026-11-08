// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ns/10ps
module shc_units(C,D);parameter S=0.016,H=0.026;input C,D;reg notifier;initial notifier=0;
 specify $setuphold(posedge C,D,S,H,notifier);endspecify endmodule
`timescale 1ps/1ps
module tb;reg C,D;shc_units p(C,D);shc_units #(0.004,0.004) z(C,D);
 initial begin C=0;D=0;#100 D=1;#19 C=1;#29 D=0;#2 D=1;end
initial begin #1;

$shc_population("tb.p", 1);
$shc_population("tb.z", 1);
$shc_watch("tb.p", "setuphold", "C", "posedge", "D", "any", 0.02, 20, 0.03, 30, "notifier", 2, 119, 148);
$shc_watch("tb.z", "setuphold", "C", "posedge", "D", "any", 0, 0, 0, 0, "notifier", 0);
$shc_start; #199; $shc_finish; $finish; end
endmodule
