// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module frozen(C,R,RE);input C,R,RE;reg nr,nf;initial begin nr=0;nf=0;end
 specify
  $recovery(posedge R &&& RE,posedge C &&& RE,144,nr);
  $recovery(negedge R &&& RE,posedge C &&& RE,173,nf);
 endspecify
endmodule

module tb;
  reg C,R,RE;frozen u(C,R,RE);
  initial begin R=0;#1000 R=1'bx;#100 R=1'bz;#100 R=1'bx;#100 R=0;#100 R=0;
    #400 R=1'bz;#100 R=1;#400 R=1'bx;#100 R=1'bz;#100 R=0;
    #400 R=1'bz;#50 R=1'bz;#50 R=1'bx;#100 R=1;
  end
  initial begin RE=0;#500 RE=1;#1200 RE=0;#300 RE=1;end
  initial begin C=0;#1500 C=1;#100 C=0;#250 C=1;#100 C=0;#150 C=1;#100 C=0;
    #473 C=1;#127 C=0;#443 C=1;#157 C=0;#1600 C=1;#400 C=0;
  end
  initial begin
    #1;

    $rr_watch("tb.u", "recovery", "R", "posedge", "C", "posedge", 144.0, 144, "nr", 1'b0, 1, 3243);

    $rr_watch("tb.u", "recovery", "R", "negedge", "C", "posedge", 173.0, 173, "nf", 1'b0, 0);

    $rr_start;
    #6999;
    $rr_finish; $finish;
  end
endmodule
