// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module renamed(DATA,ALLOW,CLK,GATE);input DATA,ALLOW,CLK,GATE;reg notifier;initial notifier=0;
 specify $setuphold(posedge CLK &&& GATE,DATA &&& (ALLOW === 1'b1),10,10,notifier);endspecify
endmodule
module tb;
reg c,d,e;renamed z(d,e,c,e),a(d,e,c,e);
initial begin c=0;d=0;e=0;#50 e=1;#50 d=1;#5 c=1;#3 d=0;end
initial begin #1;
$shc_population("tb.z",1);
$shc_watch("tb.z","setuphold","CLK","posedge","DATA","any",10,10,10,10,"notifier",1'b0,2,105,108);
$shc_population("tb.a",1);
$shc_watch("tb.a","setuphold","CLK","posedge","DATA","any",10,10,10,10,"notifier",1'b0,2,105,108);
$shc_start; #139; $shc_finish; $finish; end
endmodule
