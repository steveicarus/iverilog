// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module leaf(C,R,CE,RE);
  input C,R,CE,RE;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $recovery(R, posedge C &&& CE, 10, ns);
    $removal(R,posedge C &&& CE, 10, nh);
  endspecify
endmodule

module tb;
  reg C,R,CE; leaf u(C,R,CE,1'b1);
  initial begin C=0;R=0;CE=0;
    #20 R=1;#5 C=1;#1 R=0;#4 C=0;#10 CE=1;
    #10 R=1;#5 C=1;#1 CE=0;#2 R=0;#7 C=0;#10 C=1;#3 R=1;
  end
  initial begin
    #1;

    $rr_watch("tb.u", "recovery", "R", "any", "C", "posedge", 10.0, 10, "ns", 1, 55);

    $rr_watch("tb.u", "removal", "R", "any", "C", "posedge", 10.0, 10, "nh", 1, 58);

    $rr_start;
    #199;
    $rr_finish; $finish;
  end
endmodule
