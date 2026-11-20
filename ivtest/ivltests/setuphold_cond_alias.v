// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module split(T,R,E);input T,R,E;reg notifier;initial notifier=0;
 specify $setuphold(posedge T &&& R,posedge T &&& E,10,10,notifier);endspecify
endmodule
module selfq(T);input T;reg notifier;initial notifier=0;
 specify $setuphold(posedge T &&& T,posedge T &&& (T === 1'b1),10,10,notifier);endspecify
endmodule
module feedback(T);input T;reg notifier;initial notifier=1;
specify $setuphold(posedge T &&& notifier,posedge T &&& notifier,10,10,notifier);endspecify endmodule
module tb;
reg T,R,E;split split_u(T,R,E);selfq self_u(T);feedback feedback_u(T);
initial begin T=0;R=1;E=0;#100 T=1;#4 T=0;R=0;E=1;#1 T=1;
#3 T=0;R=1;E=0;#1 T=1;#11 T=0;E=1;#30 T=1;end
initial begin #1;
$shc_population("tb.split_u",1);
$shc_watch("tb.split_u","setuphold","T","posedge","T","posedge",10,10,10,10,"notifier",1'b0,3,105,109,150);
$shc_population("tb.self_u",1);
$shc_watch("tb.self_u","setuphold","T","posedge","T","posedge",10,10,10,10,"notifier",1'b0,4,100,105,109,150);
$shc_population("tb.feedback_u",1);
$shc_watch("tb.feedback_u","setuphold","T","posedge","T","posedge",10,10,10,10,"notifier",1'b1,1,100);
$shc_start; #169; $shc_finish; $finish; end
endmodule
