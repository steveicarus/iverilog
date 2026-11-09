// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module history_s(C,R,RE);
 input C,R,RE;reg ns;initial ns=0;
 specify $recovery(R &&& RE,posedge C,10,ns);endspecify
endmodule
module history_h(C,R,RE);
 input C,R,RE;reg nh;initial nh=0;
 specify $removal(R,posedge C &&& RE,10,nh);endspecify
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

    $rr_watch("tb.s", "recovery", "R", "any", "C", "posedge", 10.0, 10, "ns", 1'b0, 1, 104);

    $rr_watch("tb.h", "removal", "R", "any", "C", "posedge", 10.0, 10, "nh", 1'b0, 1, 104);

    $rr_start;
    #199;
    $rr_finish; $finish;
  end
endmodule
