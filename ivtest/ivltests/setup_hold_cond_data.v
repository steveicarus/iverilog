// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module leaf(C,D,R,E);
  input C,D,R,E;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $setup(D &&& E, posedge C, 10, ns);
    $hold(posedge C, D &&& E, 10, nh);
  endspecify
endmodule

module tb;
  reg C,D,E; leaf u(C,D,1'b1,E);
  initial begin C=0;D=0;E=0;
    #20 D=1;#5 C=1;#3 D=0;#2 C=0;#10 E=1;
    #10 D=1;#5 C=1;#1 E=0;#2 D=0;#2 E=1;#1 D=1;
    #4 C=0;#5 C=1;#10 D=0;
  end
  initial begin
    #1;
    $sh_watch("tb.u", "setup", "C", "posedge", "D", "any", 10.0, 10, "ns", 2, 55, 70);
    $sh_watch("tb.u", "hold", "C", "posedge", "D", "any", 10.0, 10, "nh", 1, 61);
    $sh_start;
    #199;
    $sh_finish; $finish;
  end
endmodule
