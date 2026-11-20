// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module dut(C,E,F);
  input C,E,F;reg n=0;
  specify
    $width(posedge C &&& E,5,0,n);
  endspecify
endmodule
module tb;
  reg C=0,E=0,F=1'bx;dut u(C,E,F),valid(C,E,F);
  initial begin
    #1 $sdf_annotate("ivltests/width_sdf_cond_reject.sdf",tb);
    #1;
    $pulse_watch(u.n,4,5,0);
    $pulse_watch(valid.n,4,20,2,115,145);
    $pulse_sdf_condition(u.n,1,u.E,1);
    $pulse_sdf_condition(valid.n,1,valid.E,1);
    #88 E=1;F=1;
    #70 $pulse_complete(2);$finish;
  end
  initial begin
    #100 C=1;#15 C=0;#15 C=1;#15 C=0;
  end
endmodule
