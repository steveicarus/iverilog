// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module dut(C,E,F);
  input C,E,F;
  reg n0=0,n1=0,n2=0,n3=0,n4=0,n5=0,guard=0;
  reg complement=0, logical_eq=0, case_ne=0, logical_ne=0;
  specify
    $period(posedge C &&& E,5,n0);
    $period(posedge C &&& (E === 1'b1),5,n1);
    $period(posedge C &&& (E === 1'b0),5,n2);
    $period(posedge C &&& F,5,n3);
    $period(posedge C,5,n4);
    $period(negedge C &&& E,5,n5);
    $width(posedge C &&& E,5,0,guard);
    // These qualifiers share runtime truth tables with other spellings.
    $period(posedge C &&& ~E,5,complement);
    $period(posedge C &&& (E == 1'b1),5,logical_eq);
    $period(posedge C &&& (E !== 1'b0),5,case_ne);
    $period(posedge C &&& (E != 1'b0),5,logical_ne);
  endspecify
endmodule
module tb;
  reg C=0,E=0,F=1'bx; dut u(C,E,F);
  initial begin
    #1 $sdf_annotate("ivltests/period_sdf_cond_select.sdf",tb);
    #1;
    $pulse_watch(u.n0,3,40,1,130);
    $pulse_watch(u.n1,3,20,0);
    $pulse_watch(u.n2,3,35,1,230);
    $pulse_watch(u.n3,3,50,1,130);
    $pulse_watch(u.n4,3,5,0);
    $pulse_watch(u.n5,3,31,1,145);
    $pulse_watch(u.guard,4,5,0);
    $pulse_watch(u.complement,3,5,0);
    $pulse_watch(u.logical_eq,3,5,0);
    $pulse_watch(u.case_ne,3,5,0);
    $pulse_watch(u.logical_ne,3,5,0);
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
