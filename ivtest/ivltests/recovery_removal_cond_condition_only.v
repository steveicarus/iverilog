// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module leaf(C,R,CE,RE);
  input C,R,CE,RE;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $recovery(posedge R &&& RE, posedge C &&& CE, 10, ns);
    $removal(posedge R &&& RE,posedge C &&& CE, 10, nh);
  endspecify
endmodule

module tb;
  reg C,R,CE,RE;leaf u(C,R,CE,RE);
  initial begin C=0;R=0;CE=1;RE=1;
    #100 R=1;#5 C=1;#1 CE=0;RE=0;#1 CE=1;RE=1;
    #1 CE=1'bz;RE=1'bz;#1 CE=1'bx;RE=1'bx;#1 CE=1;RE=1;R=0;#1 R=1;
  end
  initial begin
    #1;

    $rr_watch("tb.u", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "ns", 1, 105);

    $rr_watch("tb.u", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "nh", 1, 111);

    $rr_start;
    #199;
    $rr_finish; $finish;
  end
endmodule
