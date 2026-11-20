// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module earlyref(C,D,R,E);input C,D,R,E;reg notifier;initial notifier=0;
 specify $setuphold(posedge C &&& E,posedge D,10,10,notifier);endspecify
endmodule
module earlydata(C,D,R,E);input C,D,R,E;reg notifier;initial notifier=0;
 specify $setuphold(posedge C,posedge D &&& E,10,10,notifier);endspecify
endmodule
module tb;
reg C0,D0,C1,D1,E;earlyref rh(C0,D0,1'b1,1'b1);earlyref rn(C0,D0,E,E);
earlydata dh(C1,D1,1'b1,1'b1);earlydata dn(C1,D1,E,E);
initial begin C0=1;D0=0;C1=0;D1=1;E=0;#5 E=1;#4 D0=1;C1=1;end
initial begin #1;
$shc_population("tb.rh",1);
$shc_watch("tb.rh","setuphold","C","posedge","D","posedge",10,10,10,10,"notifier",1'b0,1,9);
$shc_population("tb.rn",1);
$shc_watch("tb.rn","setuphold","C","posedge","D","posedge",10,10,10,10,"notifier",1'b0,0);
$shc_population("tb.dh",1);
$shc_watch("tb.dh","setuphold","C","posedge","D","posedge",10,10,10,10,"notifier",1'b0,1,9);
$shc_population("tb.dn",1);
$shc_watch("tb.dn","setuphold","C","posedge","D","posedge",10,10,10,10,"notifier",1'b0,0);
$shc_start; #29; $shc_finish; $finish; end
endmodule
