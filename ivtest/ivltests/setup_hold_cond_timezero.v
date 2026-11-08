// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module zero_s(C,D,E);input C,D,E;reg ns;initial ns=0;
 specify $setup(posedge D &&& E,posedge C,10,ns);endspecify endmodule
module zero_h(C,D,E);input C,D,E;reg nh;initial nh=0;
 specify $hold(posedge C &&& E,posedge D,10,nh);endspecify endmodule

module tb;
  reg SC,SD,HC,HD,E;
  zero_s valid_s(SC,SD,1'b1),missing_s(SC,SD,E);
  zero_h valid_h(HC,HD,1'b1),missing_h(HC,HD,E);
  initial begin E=0;SC=0;SD=1;HC=1;HD=0;
    #9 SC=1;HD=1;#11 SD=0;HC=0;E=1;#1 SD=1;HC=1;
    #1 SC=0;HD=0;#3 SC=1;HD=1;
  end
  initial begin
    #1;
    $sh_watch("tb.valid_s", "setup", "C", "posedge", "D", "posedge", 10.0, 10, "ns", 2, 9, 25);
    $sh_watch("tb.missing_s", "setup", "C", "posedge", "D", "posedge", 10.0, 10, "ns", 1, 25);
    $sh_watch("tb.valid_h", "hold", "C", "posedge", "D", "posedge", 10.0, 10, "nh", 2, 9, 25);
    $sh_watch("tb.missing_h", "hold", "C", "posedge", "D", "posedge", 10.0, 10, "nh", 1, 25);
    $sh_start;
    #199;
    $sh_finish; $finish;
  end
endmodule
