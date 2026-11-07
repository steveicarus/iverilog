// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module setup_neg(C,D);
  input C,D;
  reg notifier;
  initial notifier=0;

  specify $setup(posedge D, negedge C, 10, notifier); endspecify
endmodule
`timescale 1ps/1ps
module hold_neg(C,D);
  input C,D;
  reg notifier;
  initial notifier=0;

  specify $hold(negedge C, negedge D, 10, notifier); endspecify
endmodule
`timescale 1ps/1ps
module setup_any(C,D);
  input C,D;
  reg notifier;
  initial notifier=0;

  specify $setup(D, C, 10, notifier); endspecify
endmodule
`timescale 1ps/1ps
module hold_any(C,D);
  input C,D;
  reg notifier;
  initial notifier=0;

  specify $hold(C, D, 10, notifier); endspecify
endmodule

`timescale 1ps/1ps
module tb;
  reg C,D; setup_neg sn(C,D); hold_neg hn(C,D); setup_any sa(C,D); hold_any ha(C,D);
  initial begin
    C=1; D=0;
    #20 D=1; #9 C=0; #1 D=0; #10 C=1; #5 D=1;
    #15 C=0; #10 D=0; #10 C=1; #5 D=1;
  end
  initial begin
    #1;

    $sh_watch("tb.sn", "setup", "C", "negedge", "D", "posedge", 10.0, 10, "notifier", 1, 29);
    $sh_watch("tb.hn", "hold", "C", "negedge", "D", "negedge", 10.0, 10, "notifier", 1, 30);
    $sh_watch("tb.sa", "setup", "C", "any", "D", "any", 10.0, 10, "notifier", 1, 29);
    $sh_watch("tb.ha", "hold", "C", "any", "D", "any", 10.0, 10, "notifier", 3, 30, 45, 85);
    $sh_start;
    #99;
    $sh_finish; $finish;
  end
endmodule
