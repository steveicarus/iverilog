// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ns/10ps
module setup_zero(C,D);
  input C,D;
  reg notifier;
  initial notifier=0;

  specify $setup(D, posedge C, 0, notifier); endspecify
endmodule
`timescale 1ns/10ps
module setup_roundzero(C,D);
  input C,D;
  reg notifier;
  initial notifier=0;

  specify $setup(D, posedge C, 0.004, notifier); endspecify
endmodule
`timescale 1ns/10ps
module setup_positive(C,D);
  input C,D;
  reg notifier;
  initial notifier=0;

  specify $setup(D, posedge C, 0.016, notifier); endspecify
endmodule
`timescale 1ns/10ps
module hold_zero(C,D);
  input C,D;
  reg notifier;
  initial notifier=0;

  specify $hold(posedge C, D, 0, notifier); endspecify
endmodule
`timescale 1ns/10ps
module hold_roundzero(C,D);
  input C,D;
  reg notifier;
  initial notifier=0;

  specify $hold(posedge C, D, 0.004, notifier); endspecify
endmodule
`timescale 1ns/10ps
module hold_positive(C,D);
  input C,D;
  reg notifier;
  initial notifier=0;

  specify $hold(posedge C, D, 0.016, notifier); endspecify
endmodule

`timescale 1ps/1ps
module tb;
  reg C,D;
  setup_zero setup_zero(C,D);
  setup_roundzero setup_roundzero(C,D);
  setup_positive setup_positive(C,D);
  hold_zero hold_zero(C,D);
  hold_roundzero hold_roundzero(C,D);
  hold_positive hold_positive(C,D);
  initial begin C=0;D=0;#100 D=1;#19 C=1;#11 D=0;end
  initial begin
    #1;

    $sh_watch("tb.setup_zero", "setup", "C", "posedge", "D", "any", 0.0, 0, "notifier", 1'b0, 0);
    $sh_watch("tb.setup_roundzero", "setup", "C", "posedge", "D", "any", 0.0, 0, "notifier", 1'b0, 0);
    $sh_watch("tb.setup_positive", "setup", "C", "posedge", "D", "any", 0.02, 20, "notifier", 1'b0, 1, 119);
    $sh_watch("tb.hold_zero", "hold", "C", "posedge", "D", "any", 0.0, 0, "notifier", 1'b0, 0);
    $sh_watch("tb.hold_roundzero", "hold", "C", "posedge", "D", "any", 0.0, 0, "notifier", 1'b0, 0);
    $sh_watch("tb.hold_positive", "hold", "C", "posedge", "D", "any", 0.02, 20, "notifier", 1'b0, 1, 130);
    $sh_start;
    #149;
    $sh_finish; $finish;
  end
endmodule
