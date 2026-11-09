// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module left(C,R,CE,RE);
  input C,R,CE,RE;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $recovery(R &&& RE === 1'b1, posedge C &&& CE === 1'b0, 10, ns);
    $removal(R &&& RE === 1'b1,posedge C &&& CE === 1'b0, 10, nh);
  endspecify
endmodule
module right(C,R,CE,RE);
  input C,R,CE,RE;
  reg ns; initial ns=0;
  reg nh; initial nh=0;
  specify
    $recovery(R &&& RE === 1'b0, posedge C &&& CE === 1'b1, 10, ns);
    $removal(R &&& RE === 1'b0,posedge C &&& CE === 1'b1, 10, nh);
  endspecify
endmodule

module tb;
  reg C,R,CE,RE; left a(C,R,CE,RE);right b(C,R,CE,RE);
  initial begin C=0;R=0;CE=0;RE=0;
    #20 R=1;#5 C=1;#3 R=0;#2 C=0;#10 RE=1;
    #10 R=1;#5 C=1;#1 CE=1;#2 R=0;#2 C=0;
    #5 RE=0;#5 R=1;#5 C=1;#3 R=0;
  end
  initial begin
    #1;

    $rr_watch("tb.a", "recovery", "R", "any", "C", "posedge", 10.0, 10, "ns", 1'b0, 1, 55);

    $rr_watch("tb.a", "removal", "R", "any", "C", "posedge", 10.0, 10, "nh", 1'b0, 1, 58);

    $rr_watch("tb.b", "recovery", "R", "any", "C", "posedge", 10.0, 10, "ns", 1'b0, 1, 75);

    $rr_watch("tb.b", "removal", "R", "any", "C", "posedge", 10.0, 10, "nh", 1'b0, 1, 78);

    $rr_start;
    #199;
    $rr_finish; $finish;
  end
endmodule
