// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module setup_leaf(C,D);
  input C,D;
  reg notifier;
  initial notifier=0;

  specify $setup(D, posedge C, 10, notifier); endspecify
endmodule

`timescale 1ps/1ps
module tb;
  reg C,D; setup_leaf u(C,D);
  initial begin
    C=0; D=0;
    #20 C=1; #1 C=0;
    #19 D=1; #9 C=1; #1 C=0;
    #20 D=0; #10 C=1; #1 C=0;
    #19 D=1; #0 C=1; #1 C=0;
    #19 D=0; #1 D=1; #8 C=1; #1 C=0;
    #20 C=1;
  end
  initial begin
    #1;

    $sh_watch("tb.u", "setup", "C", "posedge", "D", "any", 10.0, 10, "notifier", 1'b0, 2, 49, 129);
    $sh_start;
    #159;
    $sh_finish; $finish;
  end
endmodule
