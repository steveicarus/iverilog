// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
// Independently qualify both roles; reverse E/F declaration remains unchanged.
// First episode enables E/F===0; second enables E===1/F. Cold values are opposite.
`timescale 1ps/1ps
module dut(R,C,E,F);
  input R,C,E,F;
  reg ns0,nh0; initial begin ns0=0;nh0=0;end
  specify
    $recovery(R &&& E, posedge C &&& F === 1'b0, 10, ns0);
    $removal(R &&& E, posedge C &&& F === 1'b0, 10, nh0);
  endspecify
  reg ns1,nh1; initial begin ns1=0;nh1=0;end
  specify
    $recovery(R &&& E === 1'b1, posedge C &&& F, 10, ns1);
    $removal(R &&& E === 1'b1, posedge C &&& F, 10, nh1);
  endspecify
  reg ns2,nh2; initial begin ns2=0;nh2=0;end
  specify
    $recovery(R &&& F, posedge C &&& E, 10, ns2);
    $removal(R &&& F, posedge C &&& E, 10, nh2);
  endspecify
endmodule

module tb;
  reg C,R,E,F; dut u(R,C,E,F);
  initial begin C=0;R=0;E=0;F=1;
    #90 E=1;F=0;#10 R=1;#15 C=1;#15 R=0;#10 C=0;
    #50 F=1;#10 R=1;#5 C=1;#3 R=0;
  end
  initial begin
    #1;
    $sdf_annotate("ivltests/recovery_removal_sdf_cond_both.sdf",tb);
    $rr_watch("tb.u", "recovery", "R", "any", "C", "posedge", 20, 20, "ns0", 1'b0, 1, 115);
    $rr_watch("tb.u", "removal", "R", "any", "C", "posedge", 20, 20, "nh0", 1'b0, 1, 130);
    $rr_watch("tb.u", "recovery", "R", "any", "C", "posedge", 12, 12, "ns1", 1'b0, 1, 205);
    $rr_watch("tb.u", "removal", "R", "any", "C", "posedge", 12, 12, "nh1", 1'b0, 1, 208);
    $rr_watch("tb.u", "recovery", "R", "any", "C", "posedge", 10, 10, "ns2", 1'b0, 1, 205);
    $rr_watch("tb.u", "removal", "R", "any", "C", "posedge", 10, 10, "nh2", 1'b0, 1, 208);
    $tchk_condition("tb.u", "ns0", "reference", "bare", "E", 0);
    $tchk_condition("tb.u", "ns0", "data", "case", "F", 0);
    $tchk_condition("tb.u", "nh0", "reference", "bare", "E", 0);
    $tchk_condition("tb.u", "nh0", "data", "case", "F", 0);
    $tchk_condition("tb.u", "ns1", "reference", "case", "E", 1);
    $tchk_condition("tb.u", "ns1", "data", "bare", "F", 0);
    $tchk_condition("tb.u", "nh1", "reference", "case", "E", 1);
    $tchk_condition("tb.u", "nh1", "data", "bare", "F", 0);
    $tchk_condition("tb.u", "ns2", "reference", "bare", "F", 0);
    $tchk_condition("tb.u", "ns2", "data", "bare", "E", 0);
    $tchk_condition("tb.u", "nh2", "reference", "bare", "F", 0);
    $tchk_condition("tb.u", "nh2", "data", "bare", "E", 0);
    $rr_start;
    #229; $rr_finish; $finish;
  end
endmodule
