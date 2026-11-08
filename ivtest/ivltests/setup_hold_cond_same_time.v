// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module leaf(C,D,R,E);
  input C,D,R,E;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $setup(D &&& E, posedge C &&& R, 10, ns);
    $hold(posedge C &&& R, D &&& E, 10, nh);
  endspecify
endmodule
module zero(C,D,R,E);
  input C,D,R,E;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $setup(D &&& E, posedge C &&& R, 0, ns);
    $hold(posedge C &&& R, D &&& E, 0, nh);
  endspecify
endmodule

module tb;
  reg C,D,R,E;leaf u(C,D,R,E);zero z(C,D,R,E);
  initial begin C=0;D=0;R=1;E=1;
    #20 D=1;#0 C=1;#0 D=0;
    #20 C=0;#1 D=1;#19 C=1;#0 D=0;#0 C=0;#0 C=1;
    #20 E=0;#0 D=1;#0 E=1;#0 C=0;#0 C=1;#0 D=0;
    #4 C=0;#1 C=1;
  end
  initial begin
    #1;
    $sh_watch("tb.u", "setup", "C", "posedge", "D", "any", 10.0, 10, "ns", 1, 85);
    $sh_watch("tb.u", "hold", "C", "posedge", "D", "any", 10.0, 10, "nh", 3, 20, 60, 80);
    $sh_watch("tb.z", "setup", "C", "posedge", "D", "any", 0.0, 0, "ns", 0);
    $sh_watch("tb.z", "hold", "C", "posedge", "D", "any", 0.0, 0, "nh", 0);
    $sh_start;
    #199;
    $sh_finish; $finish;
  end
endmodule
