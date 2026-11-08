// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
// Explicit COND on either role binds the conditional declaration at30ps.
// The unconditional peer is unmatched and retains10ps; both remain live.
`timescale 1ps/1ps
module conditional(C,D,R,E);
  input C,D,R,E;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $setup(D &&& E, posedge C &&& R, 10, ns);
    $hold(posedge C &&& R, D &&& E, 10, nh);
  endspecify
endmodule
module unconditional(C,D,R,E);
  input C,D,R,E;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $setup(D, posedge C, 10, ns);
    $hold(posedge C, D, 10, nh);
  endspecify
endmodule

module tb;
  reg C,D,E;conditional c(C,D,E,E);unconditional u(C,D,E,E);
  initial begin C=0;D=0;E=1;
    #100 D=1;#15 C=1;#15 D=0;#10 C=0;#60 D=1;#5 C=1;#3 D=0;
  end
  initial begin
    #1;
    $sdf_annotate("ivltests/setup_hold_cond_partial.sdf",tb);
    $sh_watch("tb.c", "setup", "C", "posedge", "D", "any", 30.0, 30, "ns", 2, 115, 205);
    $sh_watch("tb.c", "hold", "C", "posedge", "D", "any", 30.0, 30, "nh", 2, 130, 208);
    $sh_watch("tb.u", "setup", "C", "posedge", "D", "any", 10.0, 10, "ns", 1, 205);
    $sh_watch("tb.u", "hold", "C", "posedge", "D", "any", 10.0, 10, "nh", 1, 208);
    $sh_start;
    #229;
    $sh_finish; $finish;
  end
endmodule
