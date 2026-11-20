// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module refedge(C,D,R,E);input C,D,R,E;reg notifier;initial notifier=0;
 specify $setuphold(posedge C &&& E,posedge D,10,10,notifier);endspecify
endmodule
module dataedge(C,D,R,E);input C,D,R,E;reg notifier;initial notifier=0;
 specify $setuphold(posedge C,posedge D &&& E,10,10,notifier);endspecify
endmodule
module tb;
reg Cr,Dr,Cd,Dd,E;refedge r(Cr,Dr,E,E);dataedge d(Cd,Dd,E,E);
initial begin Cr=0;Dr=0;Cd=0;Dd=0;E=0;
#100 Cr=1;Dd=1;#3 E=1;#1 Dr=1;Cd=1;#1 Cr=1'bx;Dd=1'bx;
#1 Cr=0;Dd=0;#1 Cr=1;Dd=1;end
initial begin #1;
$shc_population("tb.r",1);
$shc_watch("tb.r","setuphold","C","posedge","D","posedge",10,10,10,10,"notifier",1'b0,1,107);
$shc_population("tb.d",1);
$shc_watch("tb.d","setuphold","C","posedge","D","posedge",10,10,10,10,"notifier",1'b0,1,107);
$shc_start; #129; $shc_finish; $finish; end
endmodule
