// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module frozen(C,D,E);input C,D,E;reg nr,nf;initial begin nr=0;nf=0;end
 specify
  $setup(posedge D &&& E,posedge C &&& E,144,nr);
  $setup(negedge D &&& E,posedge C &&& E,173,nf);
 endspecify
endmodule

module tb;
  reg C,D,E;frozen u(C,D,E);
  initial begin D=0;#1000 D=1'bx;#100 D=1'bz;#100 D=1'bx;#100 D=0;#100 D=0;
    #400 D=1'bz;#100 D=1;#400 D=1'bx;#100 D=1'bz;#100 D=0;
    #400 D=1'bz;#50 D=1'bz;#50 D=1'bx;#100 D=1;
  end
  initial begin E=0;#500 E=1;#1200 E=0;#300 E=1;end
  initial begin C=0;#1500 C=1;#100 C=0;#250 C=1;#100 C=0;#150 C=1;#100 C=0;
    #473 C=1;#127 C=0;#443 C=1;#157 C=0;#1600 C=1;#400 C=0;
  end
  initial begin
    #1;
    $sh_watch("tb.u", "setup", "C", "posedge", "D", "posedge", 144.0, 144, "nr", 1, 3243);
    $sh_watch("tb.u", "setup", "C", "posedge", "D", "negedge", 173.0, 173, "nf", 0);
    $sh_start;
    #6999;
    $sh_finish; $finish;
  end
endmodule
