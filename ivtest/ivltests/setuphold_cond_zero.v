// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module both(C,D,R,E);input C,D,R,E;reg notifier;initial notifier=0;
 specify $setuphold(posedge C &&& E,posedge D &&& E,10,10,notifier);endspecify
endmodule
module onlyhold(C,D,R,E);input C,D,R,E;reg notifier;initial notifier=0;
 specify $setuphold(posedge C &&& E,posedge D &&& E,0,10,notifier);endspecify
endmodule
module onlysetup(C,D,R,E);input C,D,R,E;reg notifier;initial notifier=0;
 specify $setuphold(posedge C &&& E,posedge D &&& E,10,0,notifier);endspecify
endmodule
module zero(C,D,R,E);input C,D,R,E;reg notifier;initial notifier=0;
 specify $setuphold(posedge C &&& E,posedge D &&& E,0,0,notifier);endspecify
endmodule
module tb;
reg C,D,E;
both both_u(C,D,E,E);
onlyhold onlyhold_u(C,D,E,E);
onlysetup onlysetup_u(C,D,E,E);
zero zero_u(C,D,E,E);
initial begin C=0;D=0;E=1;#100 D=1;#0 C=1;#20 C=0;D=0;#80 C=1;#0 D=1;end
initial begin #1;
$shc_population("tb.both_u",1);
$shc_watch("tb.both_u","setuphold","C","posedge","D","posedge",10,10,10,10,"notifier",1'b0,2,100,200);
$shc_population("tb.onlyhold_u",1);
$shc_watch("tb.onlyhold_u","setuphold","C","posedge","D","posedge",0,0,10,10,"notifier",1'b0,1,200);
$shc_population("tb.onlysetup_u",1);
$shc_watch("tb.onlysetup_u","setuphold","C","posedge","D","posedge",10,10,0,0,"notifier",1'b0,0);
$shc_population("tb.zero_u",1);
$shc_watch("tb.zero_u","setuphold","C","posedge","D","posedge",0,0,0,0,"notifier",1'b0,0);
$shc_start; #229; $shc_finish; $finish; end
endmodule
