// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module conditional(C,R,CE,RE);
  input C,R,CE,RE;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $recovery(R &&& RE, posedge C &&& CE, 5, ns);
    $removal(R &&& RE,posedge C &&& CE, 5, nh);
  endspecify
endmodule
module unconditional(C,R,CE,RE);
  input C,R,CE,RE;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $recovery(R, posedge C, 7, ns);
    $removal(R,posedge C, 7, nh);
  endspecify
endmodule

module tb;
  reg C,R,RE;conditional c(C,R,RE,RE);unconditional u(C,R,RE,RE);
  initial begin C=0;R=0;RE=0;
    #100 R=1;#15 C=1;#3 R=0;#2 C=0;#20 RE=1;#10 R=1;#15 C=1;#3 R=0;
  end
  initial begin
    #1;
    $sdf_annotate("ivltests/recovery_removal_cond_bare.sdf",tb);

    $rr_watch("tb.c", "recovery", "R", "any", "C", "posedge", 20.0, 20, "ns", 1'b0, 1, 165);

    $rr_watch("tb.c", "removal", "R", "any", "C", "posedge", 12.0, 12, "nh", 1'b0, 1, 168);

    $rr_watch("tb.u", "recovery", "R", "any", "C", "posedge", 20.0, 20, "ns", 1'b0, 2, 115, 165);

    $rr_watch("tb.u", "removal", "R", "any", "C", "posedge", 12.0, 12, "nh", 1'b0, 2, 118, 168);

    $rr_start;
    #199;
    $rr_finish; $finish;
  end
endmodule
