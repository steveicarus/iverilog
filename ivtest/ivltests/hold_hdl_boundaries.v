// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module hold_leaf(C,D);
  input C,D;
  reg notifier;
  initial notifier=0;

  specify $hold(posedge C, D, 10, notifier); endspecify
endmodule

`timescale 1ps/1ps
module tb;
  reg C,D; hold_leaf u(C,D);
  initial begin
    C=0; D=0;
    #20 C=1; #9 D=1; #1 D=0; #1 C=0;
    #19 C=1; #10 D=1; #10 D=0; #1 C=0;
    #19 C=1; #0 D=1; #0 D=0; #1 C=0;
    #19 C=1; #9 D=1; #1 D=0;
  end
  initial begin
    #1;

    $sh_watch("tb.u", "hold", "C", "posedge", "D", "any", 10.0, 10, "notifier", 1'b0, 4, 29, 90, 90, 119);
    $sh_start;
    #129;
    $sh_finish; $finish;
  end
endmodule
