// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module history_s(C,D,E);
 input C,D,E;reg ns;initial ns=0;
 specify $setup(D &&& E,posedge C,10,ns);endspecify
endmodule
module history_h(C,D,E);
 input C,D,E;reg nh;initial nh=0;
 specify $hold(posedge C &&& E,D,10,nh);endspecify
endmodule

module tb;
  reg SC,SD,HC,HD,SE,HE;history_s s(SC,SD,SE);history_h h(HC,HD,HE);
  initial begin SC=0;SD=0;HC=0;HD=0;SE=0;HE=0;
    #20 SD=1;HC=1;#1 SE=1;HE=1;#4 SC=1;HD=1;
    #4 SE=0;HE=0;#1 SC=0;HC=0;SD=0;HD=0;
    #60 SE=1;HE=1;#10 SD=1;HC=1;#1 SE=0;HE=0;#3 SC=1;HD=1;
  end
  initial begin
    #1;
    $sh_watch("tb.s", "setup", "C", "posedge", "D", "any", 10.0, 10, "ns", 1, 104);
    $sh_watch("tb.h", "hold", "C", "posedge", "D", "any", 10.0, 10, "nh", 1, 104);
    $sh_start;
    #199;
    $sh_finish; $finish;
  end
endmodule
