// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
// Independently qualify both roles; reverse E/F declaration remains unchanged.
// First episode enables E/F===0; second enables E===1/F. Cold values are opposite.
`timescale 1ps/1ps
module dut(D,C,E,F);
  input D,C,E,F;
  reg ns0,nh0; initial begin ns0=0;nh0=0;end
  specify
    $setup(D &&& E, posedge C &&& F === 1'b0, 10, ns0);
    $hold(posedge C &&& F === 1'b0, D &&& E, 10, nh0);
  endspecify
  reg ns1,nh1; initial begin ns1=0;nh1=0;end
  specify
    $setup(D &&& E === 1'b1, posedge C &&& F, 10, ns1);
    $hold(posedge C &&& F, D &&& E === 1'b1, 10, nh1);
  endspecify
  reg ns2,nh2; initial begin ns2=0;nh2=0;end
  specify
    $setup(D &&& F, posedge C &&& E, 10, ns2);
    $hold(posedge C &&& E, D &&& F, 10, nh2);
  endspecify
endmodule

module tb;
  reg C,D,E,F; dut u(D,C,E,F);
  initial begin C=0;D=0;E=0;F=1;
    #90 E=1;F=0;#10 D=1;#15 C=1;#15 D=0;#10 C=0;
    #50 F=1;#10 D=1;#5 C=1;#3 D=0;
  end
  initial begin
    #1;
    $sdf_annotate("ivltests/setup_hold_sdf_cond_both.sdf",tb);
    $sh_watch("tb.u", "setup", "C", "posedge", "D", "any", 20, 20, "ns0", 1, 115);
    $sh_watch("tb.u", "hold", "C", "posedge", "D", "any", 20, 20, "nh0", 1, 130);
    $sh_watch("tb.u", "setup", "C", "posedge", "D", "any", 12, 12, "ns1", 1, 205);
    $sh_watch("tb.u", "hold", "C", "posedge", "D", "any", 12, 12, "nh1", 1, 208);
    $sh_watch("tb.u", "setup", "C", "posedge", "D", "any", 10, 10, "ns2", 1, 205);
    $sh_watch("tb.u", "hold", "C", "posedge", "D", "any", 10, 10, "nh2", 1, 208);
    $tchk_condition("tb.u", "ns0", "data", "bare", "E", 0);
    $tchk_condition("tb.u", "ns0", "reference", "case", "F", 0);
    $tchk_condition("tb.u", "nh0", "data", "bare", "E", 0);
    $tchk_condition("tb.u", "nh0", "reference", "case", "F", 0);
    $tchk_condition("tb.u", "ns1", "data", "case", "E", 1);
    $tchk_condition("tb.u", "ns1", "reference", "bare", "F", 0);
    $tchk_condition("tb.u", "nh1", "data", "case", "E", 1);
    $tchk_condition("tb.u", "nh1", "reference", "bare", "F", 0);
    $tchk_condition("tb.u", "ns2", "data", "bare", "F", 0);
    $tchk_condition("tb.u", "ns2", "reference", "bare", "E", 0);
    $tchk_condition("tb.u", "nh2", "data", "bare", "F", 0);
    $tchk_condition("tb.u", "nh2", "reference", "bare", "E", 0);
    $sh_start;
    #229; $sh_finish; $finish;
  end
endmodule
