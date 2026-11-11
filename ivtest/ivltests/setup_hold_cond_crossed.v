// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module left(C,D,R,E);
  input C,D,R,E;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $setup(D &&& E === 1'b1, posedge C &&& R === 1'b0, 10, ns);
    $hold(posedge C &&& R === 1'b0, D &&& E === 1'b1, 10, nh);
  endspecify
endmodule
module right(C,D,R,E);
  input C,D,R,E;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $setup(D &&& E === 1'b0, posedge C &&& R === 1'b1, 10, ns);
    $hold(posedge C &&& R === 1'b1, D &&& E === 1'b0, 10, nh);
  endspecify
endmodule

module tb;
  reg C,D,R,E; left a(C,D,R,E);right b(C,D,R,E);
  initial begin C=0;D=0;R=0;E=0;
    #20 D=1;#5 C=1;#3 D=0;#2 C=0;#10 E=1;
    #10 D=1;#5 C=1;#1 R=1;#2 D=0;#2 C=0;
    #5 E=0;#5 D=1;#5 C=1;#3 D=0;
  end
  initial begin
    #1;
    $sh_watch("tb.a", "setup", "C", "posedge", "D", "any", 10.0, 10, "ns", 1'b0, 1, 55);
    $sh_watch("tb.a", "hold", "C", "posedge", "D", "any", 10.0, 10, "nh", 1'b0, 1, 58);
    $sh_watch("tb.b", "setup", "C", "posedge", "D", "any", 10.0, 10, "ns", 1'b0, 1, 75);
    $sh_watch("tb.b", "hold", "C", "posedge", "D", "any", 10.0, 10, "nh", 1'b0, 1, 78);
    $sh_start;
    #199;
    $sh_finish; $finish;
  end
endmodule
