// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module leaf(C,R,CE,RE);
  input C,R,CE,RE;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $recovery(R &&& RE, posedge C, 10, ns);
    $removal(R &&& RE,posedge C, 10, nh);
  endspecify
endmodule

module tb;
  reg C,R,RE; leaf u(C,R,1'b1,RE);
  initial begin C=0;R=0;RE=0;
    #20 R=1;#5 C=1;#3 R=0;#2 C=0;#10 RE=1;
    #10 R=1;#5 C=1;#1 RE=0;#2 R=0;#2 RE=1;#1 R=1;
    #4 C=0;#5 C=1;#10 R=0;
  end
  initial begin
    #1;

    $rr_watch("tb.u", "recovery", "R", "any", "C", "posedge", 10.0, 10, "ns", 2, 55, 70);

    $rr_watch("tb.u", "removal", "R", "any", "C", "posedge", 10.0, 10, "nh", 1, 61);

    $rr_start;
    #199;
    $rr_finish; $finish;
  end
endmodule
