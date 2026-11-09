// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module sh_reject(C,D,OTHER);
  input C,D,OTHER;
  reg ns,nh;
  initial begin ns=0;nh=0;end
  specify $setup(D,posedge C,10,ns);$hold(posedge C,D,10,nh);endspecify
endmodule

`timescale 1ps/1ps
module tb;
  reg C,D,OTHER;
  sh_reject wrong_data(C,D,OTHER);
  sh_reject wrong_ref(C,D,OTHER);
  sh_reject wrong_edge(C,D,OTHER);
  sh_reject negative(C,D,OTHER);
  sh_reject overflow(C,D,OTHER);
  sh_reject wrong_cell(C,D,OTHER);
  sh_reject conditional(C,D,OTHER);
  sh_reject hierarchy(C,D,OTHER);
  sh_reject valid(C,D,OTHER);
  sh_reject combined(C,D,OTHER);
  initial begin C=0;D=0;OTHER=0;#100 D=1;#15 C=1;#15 D=0;end
  initial begin
    #1;
    $sdf_annotate("ivltests/setup_hold_hdl_reject.sdf",tb);
    $sh_watch("tb.wrong_data", "setup", "C", "posedge", "D", "any", 10.0, 10, "ns", 1'b0, 0);
    $sh_watch("tb.wrong_data", "hold", "C", "posedge", "D", "any", 10.0, 10, "nh", 1'b0, 0);
    $sh_watch("tb.wrong_ref", "setup", "C", "posedge", "D", "any", 10.0, 10, "ns", 1'b0, 0);
    $sh_watch("tb.wrong_ref", "hold", "C", "posedge", "D", "any", 10.0, 10, "nh", 1'b0, 0);
    $sh_watch("tb.wrong_edge", "setup", "C", "posedge", "D", "any", 10.0, 10, "ns", 1'b0, 0);
    $sh_watch("tb.wrong_edge", "hold", "C", "posedge", "D", "any", 10.0, 10, "nh", 1'b0, 0);
    $sh_watch("tb.negative", "setup", "C", "posedge", "D", "any", 10.0, 10, "ns", 1'b0, 0);
    $sh_watch("tb.negative", "hold", "C", "posedge", "D", "any", 10.0, 10, "nh", 1'b0, 0);
    $sh_watch("tb.overflow", "setup", "C", "posedge", "D", "any", 10.0, 10, "ns", 1'b0, 0);
    $sh_watch("tb.overflow", "hold", "C", "posedge", "D", "any", 10.0, 10, "nh", 1'b0, 0);
    $sh_watch("tb.wrong_cell", "setup", "C", "posedge", "D", "any", 10.0, 10, "ns", 1'b0, 0);
    $sh_watch("tb.wrong_cell", "hold", "C", "posedge", "D", "any", 10.0, 10, "nh", 1'b0, 0);
    $sh_watch("tb.conditional", "setup", "C", "posedge", "D", "any", 10.0, 10, "ns", 1'b0, 0);
    $sh_watch("tb.conditional", "hold", "C", "posedge", "D", "any", 10.0, 10, "nh", 1'b0, 0);
    $sh_watch("tb.hierarchy", "setup", "C", "posedge", "D", "any", 10.0, 10, "ns", 1'b0, 0);
    $sh_watch("tb.hierarchy", "hold", "C", "posedge", "D", "any", 10.0, 10, "nh", 1'b0, 0);
    $sh_watch("tb.valid", "setup", "C", "posedge", "D", "any", 30.0, 30, "ns", 1'b0, 1, 115);
    $sh_watch("tb.valid", "hold", "C", "posedge", "D", "any", 30.0, 30, "nh", 1'b0, 1, 130);
    $sh_watch("tb.combined", "setup", "C", "posedge", "D", "any", 10.0, 10, "ns", 1'b0, 0);
    $sh_watch("tb.combined", "hold", "C", "posedge", "D", "any", 10.0, 10, "nh", 1'b0, 0);
    $sh_start;
    #149;
    $sh_finish; $finish;
  end
endmodule
