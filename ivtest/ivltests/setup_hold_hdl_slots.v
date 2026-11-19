// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module slot_setup(C,D);
  input C,D;
  reg notifier;
  initial notifier=0;

  specify $setup(D, posedge C, 10, notifier); endspecify
endmodule
`timescale 1ps/1ps
module slot_hold(C,D);
  input C,D;
  reg notifier;
  initial notifier=0;

  specify $hold(posedge C, D, 10, notifier); endspecify
endmodule
`timescale 1ps/1ps
module alias_setup(C,D);
  input C,D;
  reg notifier;
  initial notifier=0;

  specify $setup(posedge D, posedge C, 10, notifier); endspecify
endmodule
`timescale 1ps/1ps
module alias_hold(C,D);
  input C,D;
  reg notifier;
  initial notifier=0;

  specify $hold(posedge C, posedge D, 10, notifier); endspecify
endmodule

`timescale 1ps/1ps
module tb;
  reg sc,sd,hc,hd,rc,rd,shared;
  slot_setup s(sc,sd);slot_hold h(hc,hd),reverse(rc,rd);
  alias_setup aset(shared,shared);alias_hold ahold(shared,shared);
  initial begin sc=0;sd=0;#100 sd=1;#0 sc=1;end
  initial begin hc=0;hd=0;#100 hc=1;#0 hd=1;#1 hd=0;end
  initial begin rc=0;rd=0;#100 rd=1;#0 rc=1;#1 rd=0;end
  initial begin shared=0;#100 shared=1;#1 shared=0;#99 shared=1;end
  initial begin
    #1;

    $sh_watch("tb.s", "setup", "C", "posedge", "D", "any", 10.0, 10, "notifier", 1'b0, 0);
    $sh_watch("tb.h", "hold", "C", "posedge", "D", "any", 10.0, 10, "notifier", 1'b0, 2, 100, 101);
    $sh_watch("tb.reverse", "hold", "C", "posedge", "D", "any", 10.0, 10, "notifier", 1'b0, 1, 101);
    $sh_watch("tb.aset", "setup", "C", "posedge", "D", "posedge", 10.0, 10, "notifier", 1'b0, 0);
    $sh_watch("tb.ahold", "hold", "C", "posedge", "D", "posedge", 10.0, 10, "notifier", 1'b0, 2, 100, 200);
    $sh_start;
    #219;
    $sh_finish; $finish;
  end
endmodule
