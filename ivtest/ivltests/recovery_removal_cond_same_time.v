// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module leaf(C,R,CE,RE);
  input C,R,CE,RE;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $recovery(R &&& RE, posedge C &&& CE, 10, ns);
    $removal(R &&& RE,posedge C &&& CE, 10, nh);
  endspecify
endmodule
module zero(C,R,CE,RE);
  input C,R,CE,RE;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $recovery(R &&& RE, posedge C &&& CE, 0, ns);
    $removal(R &&& RE,posedge C &&& CE, 0, nh);
  endspecify
endmodule

module tb;
  reg C,R,CE,RE;leaf u(C,R,CE,RE);zero z(C,R,CE,RE);
  initial begin C=0;R=0;CE=1;RE=1;
    #20 R=1;#0 C=1;#0 R=0;
    #20 C=0;#1 R=1;#19 C=1;#0 R=0;#0 C=0;#0 C=1;
    #20 RE=0;#0 R=1;#0 RE=1;#0 C=0;#0 C=1;#0 R=0;
    #4 C=0;#1 C=1;#3 R=1;
  end
  initial begin
    #1;

    $rr_watch("tb.u", "recovery", "R", "any", "C", "posedge", 10.0, 10, "ns", 3, 20, 60, 85);

    $rr_watch("tb.u", "removal", "R", "any", "C", "posedge", 10.0, 10, "nh", 1, 88);

    $rr_watch("tb.z", "recovery", "R", "any", "C", "posedge", 0.0, 0, "ns", 0);

    $rr_watch("tb.z", "removal", "R", "any", "C", "posedge", 0.0, 0, "nh", 0);

    $rr_start;
    #199;
    $rr_finish; $finish;
  end
endmodule
