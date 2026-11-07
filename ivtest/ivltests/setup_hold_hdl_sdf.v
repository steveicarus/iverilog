// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module sh_sdf_leaf(C,D,E,K);
  input C,D,E,K;
  reg ns,nh,ne,nk,nn;
  initial begin ns=0;nh=0;ne=0;nk=0;nn=0;end
  specify
    $setup(D,posedge C,5,ns);
    $hold(posedge C,D,5,nh);
    $setup(E,posedge C,9,ne);
    $setup(D,posedge K,9,nk);
    $setup(D,negedge C,5,nn);
  endspecify
endmodule

`timescale 1ps/1ps
module tb;
  reg C,D,E,K;sh_sdf_leaf u(C,D,E,K);
  initial begin C=0;D=0;E=0;K=0;
    #100 D=1;#15 C=1;#5 D=0;#10 C=0;#10 D=1;
    #20 C=1;#8 D=0;#12 C=0;#10 E=1;#5 C=1;#15 D=1;#5 C=0;
  end
  initial begin #114 K=1;#2 K=0;#27 K=1;end
  initial begin
    #1;
    $sdf_annotate("ivltests/setup_hold_hdl.sdf",tb);
    $sh_watch("tb.u", "setup", "C", "posedge", "D", "any", 20.0, 20, "ns", 1, 115);
    $sh_watch("tb.u", "hold", "C", "posedge", "D", "any", 8.0, 8, "nh", 1, 120);
    $sh_watch("tb.u", "setup", "C", "posedge", "E", "any", 9.0, 9, "ne", 1, 195);
    $sh_watch("tb.u", "setup", "K", "posedge", "D", "any", 9.0, 9, "nk", 1, 143);
    $sh_watch("tb.u", "setup", "C", "negedge", "D", "any", 5.0, 5, "nn", 0);
    $sh_start;
    #229;
    $sh_finish; $finish;
  end
endmodule
