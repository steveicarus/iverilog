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
  reg SC,SD,HC,HD,E;history_s s(SC,SD,E);history_h h(HC,HD,E);
  initial begin SC=0;SD=0;HC=0;HD=0;E=1;
    #100 SD=1;HC=1;#1 HC=0;#3 E=0;#1 SD=0;HC=1;#1 HC=0;
    #2 E=1;#1 SC=1;HD=1;#1 SC=0;#2 SC=1;HD=0;
  end
  initial begin
    #1;
    $sh_watch("tb.s", "setup", "C", "posedge", "D", "any", 10.0, 10, "ns", 1, 109);
    $sh_watch("tb.h", "hold", "C", "posedge", "D", "any", 10.0, 10, "nh", 1, 109);
    $sh_start;
    #199;
    $sh_finish; $finish;
  end
endmodule
