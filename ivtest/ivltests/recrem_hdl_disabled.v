// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module rrc_disabled(R,C,Q);input R,C;output Q;reg notifier=0;
 assign Q=R^C;
 specify $recrem(posedge R,posedge C,-1,2,notifier);endspecify
endmodule
module tb;reg R,C;wire Q;rrc_disabled u(R,C,Q);
 initial begin R=0;C=0;#1;$rrc_population("tb.u",0);
 #10 R=1;#0 C=1;#1;
 if(u.notifier!==0 || Q!==0) $display("FAILED disabled check");
 else $display("PASSED");$finish;end
endmodule
