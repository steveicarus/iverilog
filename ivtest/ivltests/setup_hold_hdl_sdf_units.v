// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ns/10ps
module sh_escaped(C_dummy,\C.dot ,\D\tail );
  input C_dummy,\C.dot ,\D\tail ;
  reg ns,nh;
  initial begin ns=0;nh=0;end
  specify
    $setup(\D\tail ,posedge \C.dot ,0.005,ns);
    $hold(posedge \C.dot ,\D\tail ,0.005,nh);
  endspecify
endmodule

`timescale 1ps/1ps
module tb;
  reg C,D;sh_escaped u(1'b0,C,D);
  initial begin C=0;D=0;#100 D=1;#30 C=1;#15 D=0;end
  initial begin
    #1;
    $sdf_annotate("ivltests/setup_hold_hdl_units.sdf",tb);
    $sh_watch("tb.u", "setup", "C.dot", "posedge", "D\\tail", "any", 0.04, 40, "ns", 1'b0, 1, 130);
    $sh_watch("tb.u", "hold", "C.dot", "posedge", "D\\tail", "any", 0.02, 20, "nh", 1'b0, 1, 145);
    $sh_start;
    #169;
    $sh_finish; $finish;
  end
endmodule
