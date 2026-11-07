// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module s_omitted(C,D);
  input C,D;
  reg notifier;
  initial notifier=0;

  specify $setup(D, posedge C, 10); endspecify
endmodule
`timescale 1ps/1ps
module h_omitted(C,D);
  input C,D;
  reg notifier;
  initial notifier=0;

  specify $hold(posedge C, D, 10); endspecify
endmodule

`timescale 1ps/1ps
module tb;
  reg C,D;s_omitted s(C,D);h_omitted h(C,D);
  initial begin C=0;D=0;#10 D=1;#5 C=1;#4 D=0;end
  initial begin
    #1;

    $sh_watch("tb.s", "setup", "C", "posedge", "D", "any", 10.0, 10, "", 1, 15);
    $sh_watch("tb.h", "hold", "C", "posedge", "D", "any", 10.0, 10, "", 1, 19);
    $sh_start;
    #29;
    $sh_finish; $finish;
  end
endmodule
