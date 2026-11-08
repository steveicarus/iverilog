// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module leaf(C,D,R,E);
  input C,D,R,E;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $setup(posedge D &&& E, posedge C &&& R, 10, ns);
    $hold(posedge C &&& R, posedge D &&& E, 10, nh);
  endspecify
endmodule

module tb;
  reg C,D,R,E;leaf u(C,D,R,E);
  initial begin C=0;D=0;R=1;E=1;
    #100 D=1;#5 C=1;#1 R=0;E=0;#1 R=1;E=1;
    #1 R=1'bz;E=1'bz;#1 R=1'bx;E=1'bx;#1 R=1;E=1;D=0;#1 D=1;
  end
  initial begin
    #1;
    $sh_watch("tb.u", "setup", "C", "posedge", "D", "posedge", 10.0, 10, "ns", 1, 105);
    $sh_watch("tb.u", "hold", "C", "posedge", "D", "posedge", 10.0, 10, "nh", 1, 111);
    $sh_start;
    #199;
    $sh_finish; $finish;
  end
endmodule
