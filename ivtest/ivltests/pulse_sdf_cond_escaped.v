// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module dut(\C.0 ,\E/0 );
  input \C.0 ,\E/0 ;
  reg nw=0,np=0;
  specify
    $width(posedge \C.0  &&& (\E/0  === 1'b0),5,0,nw);
    $period(posedge \C.0  &&& (\E/0  === 1'b0),5,np);
  endspecify
endmodule
module tb;
  reg C=0,E=1'bx;dut u(C,E);
  initial begin
    #1 $sdf_annotate("ivltests/pulse_sdf_cond_escaped.sdf",tb);
    #1;
    $pulse_watch(u.nw,4,20,2,115,145);
    $pulse_watch(u.np,3,40,1,130);
    $pulse_sdf_condition(u.nw,2,u.\E/0 ,0);
    $pulse_sdf_condition(u.np,2,u.\E/0 ,0);
    #88 E=0;
    #1 $pulse_sdf_condition(u.nw,2,u.\E/0 ,0);
    $pulse_sdf_condition(u.np,2,u.\E/0 ,0);
    #69 $pulse_complete(2);$finish;
  end
  initial begin
    #100 C=1;#15 C=0;#15 C=1;#15 C=0;
  end
endmodule
