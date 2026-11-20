// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module refq(C,D,R,E);input C,D,R,E;reg notifier;initial notifier=0;
 specify $setuphold(posedge C &&& E,D,10,10,notifier);endspecify
endmodule
module dataq(C,D,R,E);input C,D,R,E;reg notifier;initial notifier=0;
 specify $setuphold(posedge C,D &&& E,10,10,notifier);endspecify
endmodule
module tb;
reg Cr,Dr,Cd,Dd,Cn,Dn,E,En;refq r(Cr,Dr,E,E);dataq d(Cd,Dd,E,E);refq nohist(Cn,Dn,En,En);
initial begin Cr=0;Dr=0;Cd=0;Dd=0;Cn=0;Dn=0;E=1;En=0;
#100 Cr=1;Dd=1;Cn=1;#4 E=0;Cr=0;#1 Cr=1;Dd=0;En=1;
#3 E=1;#1 Dr=1;Cd=1;Dn=1;#1 Cd=0;#2 Dr=0;Cd=1;end
initial begin #1;
$shc_population("tb.r",1);
$shc_watch("tb.r","setuphold","C","posedge","D","any",10,10,10,10,"notifier",1'b0,1,109);
$shc_population("tb.d",1);
$shc_watch("tb.d","setuphold","C","posedge","D","any",10,10,10,10,"notifier",1'b0,1,109);
$shc_population("tb.nohist",1);
$shc_watch("tb.nohist","setuphold","C","posedge","D","any",10,10,10,10,"notifier",1'b0,0);
$shc_start; #149; $shc_finish; $finish; end
endmodule
