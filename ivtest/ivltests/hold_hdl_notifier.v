// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
primitive sh_notifier_effect(q,reset,n);
  output q;reg q;input reset,n;
  table
    1 ? : ? : 0;
    (10) ? : ? : -;
    0 * : ? : x;
  endtable
endprimitive
module sh_notifier_leaf(C,D,reset,q);
  parameter INITIAL=1'b0;
  input C,D,reset;output q;reg notifier;
  initial notifier=INITIAL;
  sh_notifier_effect effect(q,reset,notifier);
  specify $hold(posedge C,D,10,notifier);endspecify
endmodule
module sh_notifier_shared(C,D,reset,q);
  input C,D,reset;output q;reg notifier;
  initial notifier=0;
  sh_notifier_effect effect(q,reset,notifier);
  specify
    $hold(posedge C,D,10,notifier);
    $hold(posedge C,D,12,notifier);
  endspecify
endmodule
module sh_notifier_omitted(C,D);
  input C,D;
  specify $hold(posedge C,D,10);endspecify
endmodule

`timescale 1ps/1ps
module tb;
  reg C,D,reset;wire q0,q1,qx,qz,qs;
  sh_notifier_leaf #(1'b0) n0(C,D,reset,q0);
  sh_notifier_leaf #(1'b1) n1(C,D,reset,q1);
  sh_notifier_leaf #(1'bx) nx(C,D,reset,qx);
  sh_notifier_leaf #(1'bz) nz(C,D,reset,qz);
  sh_notifier_shared shared(C,D,reset,qs);
  sh_notifier_omitted omitted(C,D);
  initial begin C=0;D=0;reset=1;#2 reset=0;
    #8 C=1;#5 D=1;#1 C=0;#84 C=1;#5 D=0;
    #1;if (q0!==1'bx || q1!==1'bx || qx!==1'bx || qz!==1'b0 || qs!==1'bx) begin
      $display("FAILED notifier UDP consequences");$finish;
    end
  end
  initial begin
    #1;
    if (n0.notifier!==1'b0 || n1.notifier!==1'b1 || nx.notifier!==1'bx || nz.notifier!==1'bz || shared.notifier!==1'b0) begin
      $display("FAILED initial notifier values");$finish;
    end
    $sh_watch("tb.n0", "hold", "C", "posedge", "D", "any", 10.0, 10, "notifier", 1'b0, 2, 15, 105);
    $sh_watch("tb.n1", "hold", "C", "posedge", "D", "any", 10.0, 10, "notifier", 1'b1, 2, 15, 105);
    $sh_watch("tb.nx", "hold", "C", "posedge", "D", "any", 10.0, 10, "notifier", 1'bx, 2, 15, 105);
    $sh_watch("tb.nz", "hold", "C", "posedge", "D", "any", 10.0, 10, "notifier", 1'bz, 2, 15, 105);
    $sh_watch("tb.shared", "hold", "C", "posedge", "D", "any", 10.0, 10, "notifier", 1'b0, 2, 15, 105);
    $sh_watch("tb.shared", "hold", "C", "posedge", "D", "any", 12.0, 12, "notifier", 1'b0, 2, 15, 105);
    $sh_watch("tb.omitted", "hold", "C", "posedge", "D", "any", 10.0, 10, "", 1'b0, 2, 15, 105);
    $sh_start;
    #119;
    $sh_finish; $finish;
  end
endmodule
