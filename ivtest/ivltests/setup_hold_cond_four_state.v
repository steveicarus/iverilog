// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module bare(C,D,R,E);
  input C,D,R,E;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $setup(posedge D &&& E, posedge C &&& E, 10, ns);
    $hold(posedge C &&& E, posedge D &&& E, 10, nh);
  endspecify
endmodule
module eqzero(C,D,R,E);
  input C,D,R,E;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $setup(posedge D &&& E === 1'b0, posedge C &&& E === 1'b0, 10, ns);
    $hold(posedge C &&& E === 1'b0, posedge D &&& E === 1'b0, 10, nh);
  endspecify
endmodule
module eqone(C,D,R,E);
  input C,D,R,E;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $setup(posedge D &&& E === 1'b1, posedge C &&& E === 1'b1, 10, ns);
    $hold(posedge C &&& E === 1'b1, posedge D &&& E === 1'b1, 10, nh);
  endspecify
endmodule

module tb;
  reg C,D,E;bare b(C,D,1'b1,E);eqzero z(C,D,1'b1,E);eqone o(C,D,1'b1,E);
  initial begin C=0;D=0;E=0;
    #20 D=1;#5 C=1;#2 D=0;#1 D=1;#2 C=0;#1 D=0;
    #19 E=1;#10 D=1;#5 C=1;#2 D=0;#1 D=1;#2 C=0;#1 D=0;
    #19 E=1'bx;#10 D=1;#5 C=1;#2 D=0;#1 D=1;#2 C=0;#1 D=0;
    #19 E=1'bz;#10 D=1;#5 C=1;#2 D=0;#1 D=1;
  end
  initial begin
    #1;
    $sh_watch("tb.b", "setup", "C", "posedge", "D", "posedge", 10.0, 10, "ns", 1, 65);
    $sh_watch("tb.b", "hold", "C", "posedge", "D", "posedge", 10.0, 10, "nh", 1, 68);
    $sh_watch("tb.z", "setup", "C", "posedge", "D", "posedge", 10.0, 10, "ns", 1, 25);
    $sh_watch("tb.z", "hold", "C", "posedge", "D", "posedge", 10.0, 10, "nh", 1, 28);
    $sh_watch("tb.o", "setup", "C", "posedge", "D", "posedge", 10.0, 10, "ns", 1, 65);
    $sh_watch("tb.o", "hold", "C", "posedge", "D", "posedge", 10.0, 10, "nh", 1, 68);
    $sh_start;
    #199;
    $sh_finish; $finish;
  end
endmodule
