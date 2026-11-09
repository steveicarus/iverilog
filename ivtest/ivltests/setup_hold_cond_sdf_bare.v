// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module conditional(C,D,R,E);
  input C,D,R,E;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $setup(D &&& E, posedge C &&& R, 5, ns);
    $hold(posedge C &&& R, D &&& E, 5, nh);
  endspecify
endmodule
module unconditional(C,D,R,E);
  input C,D,R,E;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $setup(D, posedge C, 7, ns);
    $hold(posedge C, D, 7, nh);
  endspecify
endmodule

module tb;
  reg C,D,E;conditional c(C,D,E,E);unconditional u(C,D,E,E);
  initial begin C=0;D=0;E=0;
    #100 D=1;#15 C=1;#3 D=0;#2 C=0;#20 E=1;#10 D=1;#15 C=1;#3 D=0;
  end
  initial begin
    #1;
    $sdf_annotate("ivltests/setup_hold_cond_bare.sdf",tb);
    $sh_watch("tb.c", "setup", "C", "posedge", "D", "any", 20.0, 20, "ns", 1'b0, 1, 165);
    $sh_watch("tb.c", "hold", "C", "posedge", "D", "any", 12.0, 12, "nh", 1'b0, 1, 168);
    $sh_watch("tb.u", "setup", "C", "posedge", "D", "any", 20.0, 20, "ns", 1'b0, 2, 115, 165);
    $sh_watch("tb.u", "hold", "C", "posedge", "D", "any", 12.0, 12, "nh", 1'b0, 2, 118, 168);
    $sh_start;
    #199;
    $sh_finish; $finish;
  end
endmodule
