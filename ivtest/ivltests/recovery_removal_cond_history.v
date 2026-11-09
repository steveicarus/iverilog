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
  reg SC,SD,HC,HD,RE;history_s s(SC,SD,RE);history_h h(HC,HD,RE);
  initial begin SC=0;SD=0;HC=0;HD=0;RE=1;
    #100 SD=1;HC=1;#1 HC=0;#3 RE=0;#1 SD=0;HC=1;#1 HC=0;
    #2 RE=1;#1 SC=1;HD=1;#1 SC=0;#2 SC=1;HD=0;
  end
  initial begin
    #1;

    $rr_watch("tb.s", "recovery", "R", "any", "C", "posedge", 10.0, 10, "ns", 1'b0, 1, 109);

    $rr_watch("tb.h", "removal", "R", "any", "C", "posedge", 10.0, 10, "nh", 1'b0, 1, 109);

    $rr_start;
    #199;
    $rr_finish; $finish;
  end
endmodule
