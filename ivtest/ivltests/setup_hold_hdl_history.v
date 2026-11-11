// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module history_setup(C,D);
  input C,D;
  reg notifier;
  initial notifier=0;

  specify $setup(posedge D, posedge C, 10, notifier); endspecify
endmodule
`timescale 1ps/1ps
module history_hold(C,D);
  input C,D;
  reg notifier;
  initial notifier=0;

  specify $hold(posedge C, D, 10, notifier); endspecify
endmodule

`timescale 1ps/1ps
module tb;
  reg szc,szd,slc,sld,hzc,hzd,hlc,hld;
  history_setup sz(szc,szd),sl(slc,sld);
  history_hold hz(hzc,hzd),hl(hlc,hld);
  initial begin szc=0;szd=1;#9 szc=1;#1 szc=0;#9 szc=1;end
  initial begin slc=0;#20 slc=1;#1 slc=0;#79 sld=1;#9 slc=1;#1 slc=0;#10 slc=1;end
  initial begin hzc=1;#9 hzd=1;#10 hzd=0;end
  initial begin #20 hld=1;#80 hlc=1;#9 hld=0;#11 hld=1;end
  initial begin
    #1;

    $sh_watch("tb.sz", "setup", "C", "posedge", "D", "posedge", 10.0, 10, "notifier", 1'b0, 1, 9);
    $sh_watch("tb.sl", "setup", "C", "posedge", "D", "posedge", 10.0, 10, "notifier", 1'b0, 1, 109);
    $sh_watch("tb.hz", "hold", "C", "posedge", "D", "any", 10.0, 10, "notifier", 1'b0, 1, 9);
    $sh_watch("tb.hl", "hold", "C", "posedge", "D", "any", 10.0, 10, "notifier", 1'b0, 1, 109);
    $sh_start;
    #139;
    $sh_finish; $finish;
  end
endmodule
