// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module ms_any(C,D,E);input C,D,E;reg ns;initial ns=0;specify $setup(D &&& E,posedge C,10,ns);endspecify endmodule
module mh_any(C,D,E);input C,D,E;reg nh;initial nh=0;specify $hold(C &&& E,posedge D,10,nh);endspecify endmodule
module ms_pos(C,D,E);input C,D,E;reg ns;initial ns=0;specify $setup(posedge D &&& E,posedge C,10,ns);endspecify endmodule
module mh_pos(C,D,E);input C,D,E;reg nh;initial nh=0;specify $hold(posedge C &&& E,posedge D,10,nh);endspecify endmodule
module ms_neg(C,D,E);input C,D,E;reg ns;initial ns=0;specify $setup(negedge D &&& E,posedge C,10,ns);endspecify endmodule
module mh_neg(C,D,E);input C,D,E;reg nh;initial nh=0;specify $hold(negedge C &&& E,posedge D,10,nh);endspecify endmodule

module tb;
  reg SC,SD,HC,HD,E;ms_any s_any(SC,SD,E);mh_any h_any(HC,HD,E);ms_pos s_pos(SC,SD,E);mh_pos h_pos(HC,HD,E);ms_neg s_neg(SC,SD,E);mh_neg h_neg(HC,HD,E);
  initial begin SC=0;SD=0;HC=0;HD=0;E=0;
    #20 SD=1;HC=1;#1 E=1;#1 SD=1'bx;HC=1'bx;#3 SC=1;HD=1;
    #5 SC=0;HD=0;#9 E=0;#1 SD=0;HC=0;#1 E=1;#1 SD=1'bz;HC=1'bz;#3 SC=1;HD=1;
  end
  initial begin
    #1;
    $sh_watch("tb.s_any", "setup", "C", "posedge", "D", "any", 10.0, 10, "ns", 2, 25, 45);
    $sh_watch("tb.h_any", "hold", "C", "any", "D", "posedge", 10.0, 10, "nh", 2, 25, 45);
    $sh_watch("tb.s_pos", "setup", "C", "posedge", "D", "posedge", 10.0, 10, "ns", 1, 45);
    $sh_watch("tb.h_pos", "hold", "C", "posedge", "D", "posedge", 10.0, 10, "nh", 1, 45);
    $sh_watch("tb.s_neg", "setup", "C", "posedge", "D", "negedge", 10.0, 10, "ns", 1, 25);
    $sh_watch("tb.h_neg", "hold", "C", "negedge", "D", "posedge", 10.0, 10, "nh", 1, 25);
    $sh_start;
    #199;
    $sh_finish; $finish;
  end
endmodule
