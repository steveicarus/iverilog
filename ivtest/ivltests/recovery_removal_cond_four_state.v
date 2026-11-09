// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module bare(C,R,CE,RE);
  input C,R,CE,RE;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $recovery(posedge R &&& RE, posedge C &&& RE, 10, ns);
    $removal(posedge R &&& RE,posedge C &&& RE, 10, nh);
  endspecify
endmodule
module eqzero(C,R,CE,RE);
  input C,R,CE,RE;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $recovery(posedge R &&& RE === 1'b0, posedge C &&& RE === 1'b0, 10, ns);
    $removal(posedge R &&& RE === 1'b0,posedge C &&& RE === 1'b0, 10, nh);
  endspecify
endmodule
module eqone(C,R,CE,RE);
  input C,R,CE,RE;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $recovery(posedge R &&& RE === 1'b1, posedge C &&& RE === 1'b1, 10, ns);
    $removal(posedge R &&& RE === 1'b1,posedge C &&& RE === 1'b1, 10, nh);
  endspecify
endmodule

module tb;
  reg C,R,RE;bare b(C,R,1'b1,RE);eqzero z(C,R,1'b1,RE);eqone o(C,R,1'b1,RE);
  initial begin C=0;R=0;RE=0;
    #20 R=1;#5 C=1;#2 R=0;#1 R=1;#2 C=0;#1 R=0;
    #19 RE=1;#10 R=1;#5 C=1;#2 R=0;#1 R=1;#2 C=0;#1 R=0;
    #19 RE=1'bx;#10 R=1;#5 C=1;#2 R=0;#1 R=1;#2 C=0;#1 R=0;
    #19 RE=1'bz;#10 R=1;#5 C=1;#2 R=0;#1 R=1;
  end
  initial begin
    #1;

    $rr_watch("tb.b", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "ns", 1'b0, 1, 65);

    $rr_watch("tb.b", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "nh", 1'b0, 1, 68);

    $rr_watch("tb.z", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "ns", 1'b0, 1, 25);

    $rr_watch("tb.z", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "nh", 1'b0, 1, 28);

    $rr_watch("tb.o", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "ns", 1'b0, 1, 65);

    $rr_watch("tb.o", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "nh", 1'b0, 1, 68);

    $rr_start;
    #199;
    $rr_finish; $finish;
  end
endmodule
