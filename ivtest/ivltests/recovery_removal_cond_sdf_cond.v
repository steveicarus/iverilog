// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
// Explicit COND on either role binds the conditional declaration at30ps.
// The unconditional peer is unmatched and retains10ps; both remain live.
`timescale 1ps/1ps
module conditional(C,R,CE,RE);
  input C,R,CE,RE;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $recovery(R &&& RE, posedge C &&& CE, 10, ns);
    $removal(R &&& RE,posedge C &&& CE, 10, nh);
  endspecify
endmodule
module unconditional(C,R,CE,RE);
  input C,R,CE,RE;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $recovery(R, posedge C, 10, ns);
    $removal(R,posedge C, 10, nh);
  endspecify
endmodule

module tb;
  reg C,R,RE;conditional c(C,R,RE,RE);unconditional u(C,R,RE,RE);
  initial begin C=0;R=0;RE=1;
    #100 R=1;#15 C=1;#15 R=0;#10 C=0;#60 R=1;#5 C=1;#3 R=0;
  end
  initial begin
    #1;
    $sdf_annotate("ivltests/recovery_removal_cond_partial.sdf",tb);

    $rr_watch("tb.c", "recovery", "R", "any", "C", "posedge", 30.0, 30, "ns", 2, 115, 205);

    $rr_watch("tb.c", "removal", "R", "any", "C", "posedge", 30.0, 30, "nh", 2, 130, 208);

    $rr_watch("tb.u", "recovery", "R", "any", "C", "posedge", 10.0, 10, "ns", 1, 205);

    $rr_watch("tb.u", "removal", "R", "any", "C", "posedge", 10.0, 10, "nh", 1, 208);

    $rr_start;
    #229;
    $rr_finish; $finish;
  end
endmodule
