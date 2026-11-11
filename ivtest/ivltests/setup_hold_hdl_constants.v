// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module constant_leaf(C,D);
 parameter HOLD=10;
 input C,D;reg ns,nh,ns2;
 initial begin ns=0;nh=0;ns2=0;end
 specify
  specparam SETUP=10;
  $setup(D,posedge C,SETUP,ns);
  $setup(D,posedge C,SETUP,ns2);
  $hold(posedge C,D,HOLD,nh);
 endspecify
endmodule
module tb;
 reg C,D;constant_leaf u(C,D);
 initial begin C=0;D=0;#10 D=1;#5 C=1;#4 D=0;end
 initial begin
  #1;
  $sh_watch("tb.u", "setup", "C", "posedge", "D", "any", 10.0, 10, "ns", 1'b0, 1, 15);
  $sh_watch("tb.u", "hold", "C", "posedge", "D", "any", 10.0, 10, "nh", 1'b0, 1, 19);
  $sh_watch("tb.u", "setup", "C", "posedge", "D", "any", 10.0, 10, "ns2", 1'b0, 1, 15);
  $sh_start;#29;$sh_finish;$finish;
 end
endmodule
