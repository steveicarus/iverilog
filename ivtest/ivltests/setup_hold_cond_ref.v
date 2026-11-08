// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module leaf(C,D,R,E);
  input C,D,R,E;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $setup(D, posedge C &&& R, 10, ns);
    $hold(posedge C &&& R, D, 10, nh);
  endspecify
endmodule

module tb;
  reg C,D,R; leaf u(C,D,R,1'b1);
  initial begin C=0;D=0;R=0;
    #20 D=1;#5 C=1;#1 D=0;#4 C=0;#10 R=1;
    #10 D=1;#5 C=1;#1 R=0;#2 D=0;#7 C=0;#10 C=1;#3 D=1;
  end
  initial begin
    #1;
    $sh_watch("tb.u", "setup", "C", "posedge", "D", "any", 10.0, 10, "ns", 1, 55);
    $sh_watch("tb.u", "hold", "C", "posedge", "D", "any", 10.0, 10, "nh", 1, 58);
    $sh_start;
    #199;
    $sh_finish; $finish;
  end
endmodule
