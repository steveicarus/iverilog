// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module zero_s(C,R,RE);input C,R,RE;reg ns;initial ns=0;
 specify $recovery(posedge R &&& RE,posedge C,10,ns);endspecify endmodule
module zero_h(C,R,RE);input C,R,RE;reg nh;initial nh=0;
 specify $removal(posedge R,posedge C &&& RE,10,nh);endspecify endmodule

module tb;
  reg SC,SD,HC,HD,RE;
  zero_s valid_s(SC,SD,1'b1),missing_s(SC,SD,RE);
  zero_h valid_h(HC,HD,1'b1),missing_h(HC,HD,RE);
  initial begin RE=0;SC=0;SD=1;HC=1;HD=0;
    #9 SC=1;HD=1;#11 SD=0;HC=0;RE=1;#1 SD=1;HC=1;
    #1 SC=0;HD=0;#3 SC=1;HD=1;
  end
  initial begin
    #1;

    $rr_watch("tb.valid_s", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "ns", 1'b0, 2, 9, 25);

    $rr_watch("tb.missing_s", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "ns", 1'b0, 1, 25);

    $rr_watch("tb.valid_h", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "nh", 1'b0, 2, 9, 25);

    $rr_watch("tb.missing_h", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "nh", 1'b0, 1, 25);

    $rr_start;
    #199;
    $rr_finish; $finish;
  end
endmodule
