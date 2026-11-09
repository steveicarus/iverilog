// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
primitive rr_notifier_effect(q,reset,n);
 output q;reg q;input reset,n;
 table
  1 ? : ? : 0;
  (10) ? : ? : -;
  0 * : ? : x;
 endtable
endprimitive
module rr_notify(R,C,reset,q);
 parameter INITIAL=0;input R,C,reset;output q;reg notifier;initial notifier=INITIAL;
 rr_notifier_effect effect(q,reset,notifier);
 specify $removal(posedge R,posedge C,10,notifier);endspecify
endmodule
module rr_shared(R,C,reset,q);
 input R,C,reset;output q;reg notifier;initial notifier=0;
 rr_notifier_effect effect(q,reset,notifier);
 specify $removal(posedge R,posedge C,10,notifier);$removal(posedge R,posedge C,12,notifier);endspecify
endmodule
module rr_omitted(R,C);
 input R,C;specify $removal(posedge R,posedge C,10);endspecify
endmodule
module tb;
 reg R,C,reset;wire q0,q1,qx,qz,qs;
 rr_notify #(1'b0) n0(R,C,reset,q0);rr_notify #(1'b1) n1(R,C,reset,q1);
 rr_notify #(1'bx) nx(R,C,reset,qx);rr_notify #(1'bz) nz(R,C,reset,qz);
 rr_shared shared(R,C,reset,qs);rr_omitted omitted(R,C);
 initial begin R=0;C=0;reset=1;#2 reset=0;#8 C=1;#5 R=1;#1 begin R=0;C=0;end #84 C=1;#5 R=1;
  #1;if(q0!==1'bx || q1!==1'bx || qx!==1'bx || qz!==1'b0 || qs!==1'bx) begin
   $display("FAILED notifier UDP consequences");$finish;end
 end
 initial begin #1;
  if(n0.notifier!==1'b0 || n1.notifier!==1'b1 || nx.notifier!==1'bx || nz.notifier!==1'bz || shared.notifier!==1'b0) begin
   $display("FAILED initial notifier values");$finish;end
    $rr_watch("tb.n0", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "notifier", 1'b0, 2, 15, 105);
    $rr_watch("tb.n1", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "notifier", 1'b1, 2, 15, 105);
    $rr_watch("tb.nx", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "notifier", 1'bx, 2, 15, 105);
    $rr_watch("tb.nz", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "notifier", 1'bz, 2, 15, 105);
    $rr_watch("tb.shared", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "notifier", 1'b0, 2, 15, 105);
    $rr_watch("tb.shared", "removal", "R", "posedge", "C", "posedge", 12.0, 12, "notifier", 1'b0, 2, 15, 105);
    $rr_watch("tb.omitted", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "", 1'b0, 2, 15, 105);
  $rr_start;#119;$rr_finish;$finish;
 end
endmodule
