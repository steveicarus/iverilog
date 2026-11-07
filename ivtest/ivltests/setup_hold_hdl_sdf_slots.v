// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
// Independent selected-slot windows: SETUP25/35/45, HOLD10/20/30 ps.
`timescale 1ps/1ps
module sh_slot_leaf(C,D);
 input C,D;reg ns,nh;
 initial begin ns=0;nh=0;end
 specify $setup(D,posedge C,5,ns);$hold(posedge C,D,5,nh);endspecify
endmodule
module tb;
 reg C,D;sh_slot_leaf u(C,D);
 initial begin
  C=0;D=0;
  #100 D=1;#30 C=1;#19 D=0;#1 C=0;
  #50 D=1;#20 C=1;#5 D=0;#5 C=0;
  #30 D=1;#40 C=1;#25 D=0;#5 C=0;
  #70 D=1;#45 C=1;#30 D=0;
 end
 initial begin
  #1;$sdf_annotate("ivltests/setup_hold_hdl_slots.sdf",tb);
`ifdef SH_SLOT_MIN
  $sh_watch("tb.u","setup","C","posedge","D","any",25.0,25,"ns",1,220);
  $sh_watch("tb.u","hold","C","posedge","D","any",10.0,10,"nh",1,225);
`elsif SH_SLOT_MAX
  $sh_watch("tb.u","setup","C","posedge","D","any",45.0,45,"ns",3,130,220,300);
  $sh_watch("tb.u","hold","C","posedge","D","any",30.0,30,"nh",3,149,225,325);
`else
  $sh_watch("tb.u","setup","C","posedge","D","any",35.0,35,"ns",2,130,220);
  $sh_watch("tb.u","hold","C","posedge","D","any",20.0,20,"nh",2,149,225);
`endif
  $sh_start;#499;$sh_finish;$finish;
 end
endmodule
