// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module shared(C,R,CE,RE);
  input C,R,CE,RE;
  reg n; initial n=0;
  specify
    $recovery(R &&& RE, posedge C &&& CE, 10, n);
    $removal(R &&& RE,posedge C &&& CE, 10, n);
  endspecify
endmodule
module omitted(C,R,CE,RE);
  input C,R,CE,RE;
  specify
    $recovery(R &&& RE, posedge C &&& CE, 10);
    $removal(R &&& RE,posedge C &&& CE, 10);
  endspecify
endmodule
module highz(C,R,CE,RE);
  input C,R,CE,RE;
  reg n; initial n=1'bz;
  specify
    $recovery(R &&& RE, posedge C &&& CE, 10, n);
    $removal(R &&& RE,posedge C &&& CE, 10, n);
  endspecify
endmodule

module tb;
  reg C,R,RE;shared shared(C,R,RE,RE);omitted omitted(C,R,RE,RE);highz highz(C,R,RE,RE);
  initial begin C=0;R=0;RE=0;#20 R=1;#5 C=1;#3 R=0;#2 C=0;
    #60 RE=1;#10 R=1;#5 C=1;#3 R=0;
  end
  initial begin
    #1;

    $rr_watch("tb.shared", "recovery", "R", "any", "C", "posedge", 10.0, 10, "n", 1, 105);

    $rr_watch("tb.shared", "removal", "R", "any", "C", "posedge", 10.0, 10, "n", 1, 108);

    $rr_watch("tb.omitted", "recovery", "R", "any", "C", "posedge", 10.0, 10, "", 1, 105);

    $rr_watch("tb.omitted", "removal", "R", "any", "C", "posedge", 10.0, 10, "", 1, 108);

    $rr_watch("tb.highz", "recovery", "R", "any", "C", "posedge", 10.0, 10, "n", 1, 105);

    $rr_watch("tb.highz", "removal", "R", "any", "C", "posedge", 10.0, 10, "n", 1, 108);

    $rr_start;
    #199;
    $rr_finish; $finish;
  end
endmodule
