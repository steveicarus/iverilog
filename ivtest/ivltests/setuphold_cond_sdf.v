// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module sdfcell(C,D,E);input C,D,E;reg notifier;initial notifier=0;
 specify $setuphold(posedge C &&& E,D &&& E,5,6,notifier);endspecify
endmodule
module tb;
reg C,D,E;sdfcell a(C,D,E),b(C,D,E);
initial begin C=0;D=0;E=0;#50 E=1;#50 D=1;#15 C=1;#5 D=0;end
initial begin #1;
$sdf_annotate("ivltests/setuphold_cond_bare.sdf",tb);
$shc_population("tb.a",1);
$shc_watch("tb.a","setuphold","C","posedge","D","any",20,20,8,8,"notifier",1'b0,2,115,120);
$shc_population("tb.b",1);
$shc_watch("tb.b","setuphold","C","posedge","D","any",20,20,8,8,"notifier",1'b0,2,115,120);
$shc_start; #149; $shc_finish; $finish; end
endmodule
