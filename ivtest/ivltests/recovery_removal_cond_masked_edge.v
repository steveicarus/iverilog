// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module ms_any(C,R,RE);input C,R,RE;reg ns;initial ns=0;specify $recovery(R &&& RE,posedge C,10,ns);endspecify endmodule
module mh_any(C,R,RE);input C,R,RE;reg nh;initial nh=0;specify $removal(posedge R,C &&& RE,10,nh);endspecify endmodule
module ms_pos(C,R,RE);input C,R,RE;reg ns;initial ns=0;specify $recovery(posedge R &&& RE,posedge C,10,ns);endspecify endmodule
module mh_pos(C,R,RE);input C,R,RE;reg nh;initial nh=0;specify $removal(posedge R,posedge C &&& RE,10,nh);endspecify endmodule
module ms_neg(C,R,RE);input C,R,RE;reg ns;initial ns=0;specify $recovery(negedge R &&& RE,posedge C,10,ns);endspecify endmodule
module mh_neg(C,R,RE);input C,R,RE;reg nh;initial nh=0;specify $removal(posedge R,negedge C &&& RE,10,nh);endspecify endmodule

module tb;
  reg SC,SD,HC,HD,RE;ms_any s_any(SC,SD,RE);mh_any h_any(HC,HD,RE);ms_pos s_pos(SC,SD,RE);mh_pos h_pos(HC,HD,RE);ms_neg s_neg(SC,SD,RE);mh_neg h_neg(HC,HD,RE);
  initial begin SC=0;SD=0;HC=0;HD=0;RE=0;
    #20 SD=1;HC=1;#1 RE=1;#1 SD=1'bx;HC=1'bx;#3 SC=1;HD=1;
    #5 SC=0;HD=0;#9 RE=0;#1 SD=0;HC=0;#1 RE=1;#1 SD=1'bz;HC=1'bz;#3 SC=1;HD=1;
  end
  initial begin
    #1;

    $rr_watch("tb.s_any", "recovery", "R", "any", "C", "posedge", 10.0, 10, "ns", 1'b0, 2, 25, 45);

    $rr_watch("tb.h_any", "removal", "R", "posedge", "C", "any", 10.0, 10, "nh", 1'b0, 2, 25, 45);

    $rr_watch("tb.s_pos", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "ns", 1'b0, 1, 45);

    $rr_watch("tb.h_pos", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "nh", 1'b0, 1, 45);

    $rr_watch("tb.s_neg", "recovery", "R", "negedge", "C", "posedge", 10.0, 10, "ns", 1'b0, 1, 25);

    $rr_watch("tb.h_neg", "removal", "R", "posedge", "C", "negedge", 10.0, 10, "nh", 1'b0, 1, 25);

    $rr_start;
    #199;
    $rr_finish; $finish;
  end
endmodule
