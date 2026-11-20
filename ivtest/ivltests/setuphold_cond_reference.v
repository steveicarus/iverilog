// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module ucheck(C,D,R,E);input C,D,R,E;reg notifier;initial notifier=0;
 specify $setuphold(posedge C &&& R,D,10,10,notifier);endspecify
endmodule
module tb;
reg C,D,R,E;ucheck u(C,D,R,E);
initial begin C=0;D=0;R=0;E=0;
#20 D=1;#5 C=1;#1 D=0;#4 C=0;#10 R=1;#10 D=1;#5 C=1;
#1 R=0;#2 D=0;#7 C=0;#10 C=1;#3 D=1;end
initial begin #1;
$shc_population("tb.u",1);
$shc_watch("tb.u","setuphold","C","posedge","D","any",10,10,10,10,"notifier",1'b0,2,55,58);
$shc_start; #99; $shc_finish; $finish; end
endmodule
