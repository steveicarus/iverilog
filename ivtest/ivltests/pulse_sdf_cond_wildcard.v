// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module dut(C,E,F);
  input C,E,F;
  reg w0=0,w1=0,w2=0,w3=0,w4=0,w5=0,p0=0,p1=0,p2=0,p3=0,p4=0,p5=0;
  specify
    $width(posedge C &&& E,5,0,w0);
    $width(posedge C &&& (E === 1'b1),5,0,w1);
    $width(posedge C &&& (E === 1'b0),5,0,w2);
    $width(posedge C &&& F,5,0,w3);
    $width(posedge C,5,0,w4);
    $width(negedge C &&& E,5,0,w5);
    $period(posedge C &&& E,5,p0);
    $period(posedge C &&& (E === 1'b1),5,p1);
    $period(posedge C &&& (E === 1'b0),5,p2);
    $period(posedge C &&& F,5,p3);
    $period(posedge C,5,p4);
    $period(negedge C &&& E,5,p5);
  endspecify
endmodule
module tb;
  reg C=0,E=0,F=1'bx; dut u(C,E,F);
  initial begin
    #1 $sdf_annotate("ivltests/pulse_sdf_cond_wildcard_edge.sdf",tb);
    $pulse_sdf_limit(u.w0,4,20);
    $pulse_sdf_limit(u.w1,4,20);
    $pulse_sdf_limit(u.w2,4,20);
    $pulse_sdf_limit(u.w3,4,20);
    $pulse_sdf_limit(u.w4,4,20);
    $pulse_sdf_limit(u.w5,4,5);
    $pulse_sdf_limit(u.p0,3,40);
    $pulse_sdf_limit(u.p1,3,40);
    $pulse_sdf_limit(u.p2,3,40);
    $pulse_sdf_limit(u.p3,3,40);
    $pulse_sdf_limit(u.p4,3,40);
    $pulse_sdf_limit(u.p5,3,5);
    #1 $sdf_annotate("ivltests/pulse_sdf_cond_wildcard.sdf",tb);
    $pulse_watch(u.w0,4,12,1,110);
    $pulse_watch(u.w1,4,12,1,110);
    $pulse_watch(u.w2,4,12,0);
    $pulse_watch(u.w3,4,12,1,110);
    $pulse_watch(u.w4,4,12,1,110);
    $pulse_watch(u.w5,4,12,0);
    $pulse_watch(u.p0,3,40,1,130);
    $pulse_watch(u.p1,3,40,1,130);
    $pulse_watch(u.p2,3,40,0);
    $pulse_watch(u.p3,3,40,1,130);
    $pulse_watch(u.p4,3,40,1,130);
    $pulse_watch(u.p5,3,40,1,145);
    #88 E=1;F=1;
    #70 $pulse_complete(12);$finish;
  end
  initial begin
    #100 C=1;#10 C=0;#20 C=1;#15 C=0;
  end
endmodule
