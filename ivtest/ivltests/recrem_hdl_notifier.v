// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
primitive rrc_notifier_effect(q,reset,n);
  output q;reg q;input reset,n;
  table
    1 ? : ? : 0;
    (10) ? : ? : -;
    0 * : ? : x;
  endtable
endprimitive
module rrc_notifier_leaf(C,D,reset,q);
  parameter INITIAL=1'b0;
  input C,D,reset;output q;reg notifier;
  initial notifier=INITIAL;
  rrc_notifier_effect effect(q,reset,notifier);
  specify $recrem(D,posedge C,10,6,notifier);endspecify
endmodule
module rrc_notifier_shared(C,D,reset,q);
  input C,D,reset;output q;reg notifier;
  initial notifier=0;
  rrc_notifier_effect effect(q,reset,notifier);
  specify
    $recrem(D,posedge C,10,6,notifier);
    $recrem(D,posedge C,12,6,notifier);
  endspecify
endmodule
module rrc_notifier_omitted(C,D);
  input C,D;
  specify $recrem(D,posedge C,10,6);endspecify
endmodule

`timescale 1ps/1ps
module tb;
  reg C,D,reset;wire q0,q1,qx,qz,qs;
  rrc_notifier_leaf #(1'b0) n0(C,D,reset,q0);
  rrc_notifier_leaf #(1'b1) n1(C,D,reset,q1);
  rrc_notifier_leaf #(1'bx) nx(C,D,reset,qx);
  rrc_notifier_leaf #(1'bz) nz(C,D,reset,qz);
  rrc_notifier_shared shared(C,D,reset,qs);
  rrc_notifier_omitted omitted(C,D);
  initial begin C=0;D=0;reset=1;#2 reset=0;
    #8 D=1;#5 C=1;#1 C=0;#84 D=0;#5 C=1;
    #1;if (q0!==1'bx || q1!==1'bx || qx!==1'bx || qz!==1'b0 || qs!==1'bx) begin
      $display("FAILED notifier UDP consequences");$finish;
    end
  end
  initial begin
    #1;
    if (n0.notifier!==1'b0 || n1.notifier!==1'b1 || nx.notifier!==1'bx || nz.notifier!==1'bz || shared.notifier!==1'b0) begin
      $display("FAILED initial notifier values");$finish;
    end
    $rrc_watch("tb.n0", "recrem", "D", "any", "C", "posedge", 10.0, 10, 6.0, 6, "notifier", 2, 15, 105);
    $rrc_watch("tb.n1", "recrem", "D", "any", "C", "posedge", 10.0, 10, 6.0, 6, "notifier", 2, 15, 105);
    $rrc_watch("tb.nx", "recrem", "D", "any", "C", "posedge", 10.0, 10, 6.0, 6, "notifier", 2, 15, 105);
    $rrc_watch("tb.nz", "recrem", "D", "any", "C", "posedge", 10.0, 10, 6.0, 6, "notifier", 2, 15, 105);
    $rrc_watch("tb.shared", "recrem", "D", "any", "C", "posedge", 10.0, 10, 6.0, 6, "notifier", 2, 15, 105);
    $rrc_watch("tb.shared", "recrem", "D", "any", "C", "posedge", 12.0, 12, 6.0, 6, "notifier", 2, 15, 105);
    $rrc_watch("tb.omitted", "recrem", "D", "any", "C", "posedge", 10.0, 10, 6.0, 6, "", 2, 15, 105);
    $rrc_population("tb.n0", 1);
    $rrc_population("tb.n1", 1);
    $rrc_population("tb.nx", 1);
    $rrc_population("tb.nz", 1);
    $rrc_population("tb.shared", 2);
    $rrc_population("tb.omitted", 1);
    $rrc_start;
    #119;
    $rrc_finish; $finish;
  end
endmodule
