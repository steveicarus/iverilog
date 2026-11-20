// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module dut(C,E,F);
  input C,E,F;
  reg n0=0,n1=0,n2=0,n3=0,n4=0,n5=0,guard=0;
  reg complement=0, logical_eq=0, case_ne=0, logical_ne=0;
  specify
    $width(posedge C &&& E,5,0,n0);
    $width(posedge C &&& (E === 1'b1),5,0,n1);
    $width(posedge C &&& (E === 1'b0),5,0,n2);
    $width(posedge C &&& F,5,0,n3);
    $width(posedge C,5,0,n4);
    $width(negedge C &&& E,5,0,n5);
    $period(posedge C &&& E,5,guard);
    // These qualifiers share runtime truth tables with other spellings.
    $width(posedge C &&& ~E,5,0,complement);
    $width(posedge C &&& (E == 1'b1),5,0,logical_eq);
    $width(posedge C &&& (E !== 1'b0),5,0,case_ne);
    $width(posedge C &&& (E != 1'b0),5,0,logical_ne);
  endspecify
endmodule
module tb;
  reg C=0,E=0,F=1'bx; dut u(C,E,F);
  initial begin
    #1 $sdf_annotate("ivltests/width_sdf_cond_select.sdf",tb);
    #1;
    $pulse_watch(u.n0,4,20,2,115,145);
    $pulse_watch(u.n1,4,10,0);
    $pulse_watch(u.n2,4,18,2,215,245);
    $pulse_watch(u.n3,4,25,2,115,145);
    $pulse_watch(u.n4,4,5,0);
    $pulse_watch(u.n5,4,25,1,130);
    $pulse_watch(u.guard,3,5,0);
    $pulse_watch(u.complement,4,5,0);
    $pulse_watch(u.logical_eq,4,5,0);
    $pulse_watch(u.case_ne,4,5,0);
    $pulse_watch(u.logical_ne,4,5,0);
    $pulse_sdf_condition(u.n0,1,u.E,1);
    $pulse_sdf_condition(u.n1,2,u.E,1);
    $pulse_sdf_condition(u.n2,2,u.E,0);
    $pulse_sdf_condition(u.n3,1,u.F,1);
    $pulse_sdf_condition(u.n4,0,u.E,0);
    $pulse_sdf_condition(u.n5,1,u.E,1);
    $pulse_sdf_condition(u.guard,1,u.E,1);
    #88 E=1; F=1;
    #100 E=0; F=0;
    #70 $pulse_complete(11); $finish;
  end
  initial begin
    #100 C=1; #15 C=0; #15 C=1; #15 C=0;
    #55 C=1; #15 C=0; #15 C=1; #15 C=0;
  end
endmodule
