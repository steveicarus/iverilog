// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
// Same endpoints: bare E, case-equality E/1 and E/0, other F, and unqualified.
// Values are bound while E=0/F=X. At 100/115/130 E=1; at 200/205/208 E=0.
// Limit 20 detects the first 15ps pair, limit 12 does not; case0 limit 30 detects the second.
`timescale 1ps/1ps
module dut(R,C,E,F);
  input R,C,E,F;
  reg ns0,nh0; initial begin ns0=0;nh0=0;end
  specify
    $recovery(R &&& E, posedge C &&& F, 10, ns0);
    $removal(R &&& E, posedge C &&& F, 10, nh0);
  endspecify
  reg ns1,nh1; initial begin ns1=0;nh1=0;end
  specify
    $recovery(R &&& E === 1'b1, posedge C, 10, ns1);
    $removal(R &&& E === 1'b1, posedge C, 10, nh1);
  endspecify
  reg ns2,nh2; initial begin ns2=0;nh2=0;end
  specify
    $recovery(R &&& E === 1'b0, posedge C, 10, ns2);
    $removal(R &&& E === 1'b0, posedge C, 10, nh2);
  endspecify
  reg ns3,nh3; initial begin ns3=0;nh3=0;end
  specify
    $recovery(R &&& F, posedge C, 10, ns3);
    $removal(R &&& F, posedge C, 10, nh3);
  endspecify
  reg ns4,nh4; initial begin ns4=0;nh4=0;end
  specify
    $recovery(R, posedge C, 10, ns4);
    $removal(R, posedge C, 10, nh4);
  endspecify
endmodule

module tb;
  reg C,R,E,F; dut u(R,C,E,F);
  initial begin C=0;R=0;E=0;F=1'bx;
    #90 E=1;F=1;#10 R=1;#15 C=1;#15 R=0;#10 C=0;
    #50 E=0;#10 R=1;#5 C=1;#3 R=0;
  end
  initial begin
    #1;
    $sdf_annotate("ivltests/recovery_removal_sdf_cond_select.sdf",tb);
    $rr_watch("tb.u", "recovery", "R", "any", "C", "posedge", 20, 20, "ns0", 1, 115);
    $rr_watch("tb.u", "removal", "R", "any", "C", "posedge", 20, 20, "nh0", 1, 130);
    $rr_watch("tb.u", "recovery", "R", "any", "C", "posedge", 12, 12, "ns1", 0);
    $rr_watch("tb.u", "removal", "R", "any", "C", "posedge", 12, 12, "nh1", 0);
    $rr_watch("tb.u", "recovery", "R", "any", "C", "posedge", 30, 30, "ns2", 1, 205);
    $rr_watch("tb.u", "removal", "R", "any", "C", "posedge", 30, 30, "nh2", 1, 208);
    $rr_watch("tb.u", "recovery", "R", "any", "C", "posedge", 10, 10, "ns3", 1, 205);
    $rr_watch("tb.u", "removal", "R", "any", "C", "posedge", 10, 10, "nh3", 1, 208);
    $rr_watch("tb.u", "recovery", "R", "any", "C", "posedge", 10, 10, "ns4", 1, 205);
    $rr_watch("tb.u", "removal", "R", "any", "C", "posedge", 10, 10, "nh4", 1, 208);
    $tchk_condition("tb.u", "ns0", "reference", "bare", "E", 0);
    $tchk_condition("tb.u", "ns0", "data", "bare", "F", 0);
    $tchk_condition("tb.u", "nh0", "reference", "bare", "E", 0);
    $tchk_condition("tb.u", "nh0", "data", "bare", "F", 0);
    $tchk_condition("tb.u", "ns1", "reference", "case", "E", 1);
    $tchk_condition("tb.u", "ns1", "data", "none", "", 0);
    $tchk_condition("tb.u", "nh1", "reference", "case", "E", 1);
    $tchk_condition("tb.u", "nh1", "data", "none", "", 0);
    $tchk_condition("tb.u", "ns2", "reference", "case", "E", 0);
    $tchk_condition("tb.u", "ns2", "data", "none", "", 0);
    $tchk_condition("tb.u", "nh2", "reference", "case", "E", 0);
    $tchk_condition("tb.u", "nh2", "data", "none", "", 0);
    $tchk_condition("tb.u", "ns3", "reference", "bare", "F", 0);
    $tchk_condition("tb.u", "ns3", "data", "none", "", 0);
    $tchk_condition("tb.u", "nh3", "reference", "bare", "F", 0);
    $tchk_condition("tb.u", "nh3", "data", "none", "", 0);
    $tchk_condition("tb.u", "ns4", "reference", "none", "", 0);
    $tchk_condition("tb.u", "ns4", "data", "none", "", 0);
    $tchk_condition("tb.u", "nh4", "reference", "none", "", 0);
    $tchk_condition("tb.u", "nh4", "data", "none", "", 0);
    $rr_start;
    // Probe expression values at X and Z without creating either timing event.
    #1 E=1'bx;F=1'bz;#0;
    $tchk_condition("tb.u", "ns0", "reference", "bare", "E", 0);
    $tchk_condition("tb.u", "ns0", "data", "bare", "F", 0);
    $tchk_condition("tb.u", "nh0", "reference", "bare", "E", 0);
    $tchk_condition("tb.u", "nh0", "data", "bare", "F", 0);
    $tchk_condition("tb.u", "ns1", "reference", "case", "E", 1);
    $tchk_condition("tb.u", "ns1", "data", "none", "", 0);
    $tchk_condition("tb.u", "nh1", "reference", "case", "E", 1);
    $tchk_condition("tb.u", "nh1", "data", "none", "", 0);
    $tchk_condition("tb.u", "ns2", "reference", "case", "E", 0);
    $tchk_condition("tb.u", "ns2", "data", "none", "", 0);
    $tchk_condition("tb.u", "nh2", "reference", "case", "E", 0);
    $tchk_condition("tb.u", "nh2", "data", "none", "", 0);
    $tchk_condition("tb.u", "ns3", "reference", "bare", "F", 0);
    $tchk_condition("tb.u", "ns3", "data", "none", "", 0);
    $tchk_condition("tb.u", "nh3", "reference", "bare", "F", 0);
    $tchk_condition("tb.u", "nh3", "data", "none", "", 0);
    $tchk_condition("tb.u", "ns4", "reference", "none", "", 0);
    $tchk_condition("tb.u", "ns4", "data", "none", "", 0);
    $tchk_condition("tb.u", "nh4", "reference", "none", "", 0);
    $tchk_condition("tb.u", "nh4", "data", "none", "", 0);
    #1 E=1'bz;F=1'bx;#0;
    $tchk_condition("tb.u", "ns0", "reference", "bare", "E", 0);
    $tchk_condition("tb.u", "ns0", "data", "bare", "F", 0);
    $tchk_condition("tb.u", "nh0", "reference", "bare", "E", 0);
    $tchk_condition("tb.u", "nh0", "data", "bare", "F", 0);
    $tchk_condition("tb.u", "ns1", "reference", "case", "E", 1);
    $tchk_condition("tb.u", "ns1", "data", "none", "", 0);
    $tchk_condition("tb.u", "nh1", "reference", "case", "E", 1);
    $tchk_condition("tb.u", "nh1", "data", "none", "", 0);
    $tchk_condition("tb.u", "ns2", "reference", "case", "E", 0);
    $tchk_condition("tb.u", "ns2", "data", "none", "", 0);
    $tchk_condition("tb.u", "nh2", "reference", "case", "E", 0);
    $tchk_condition("tb.u", "nh2", "data", "none", "", 0);
    $tchk_condition("tb.u", "ns3", "reference", "bare", "F", 0);
    $tchk_condition("tb.u", "ns3", "data", "none", "", 0);
    $tchk_condition("tb.u", "nh3", "reference", "bare", "F", 0);
    $tchk_condition("tb.u", "nh3", "data", "none", "", 0);
    $tchk_condition("tb.u", "ns4", "reference", "none", "", 0);
    $tchk_condition("tb.u", "ns4", "data", "none", "", 0);
    $tchk_condition("tb.u", "nh4", "reference", "none", "", 0);
    $tchk_condition("tb.u", "nh4", "data", "none", "", 0);
    #227; $rr_finish; $finish;
  end
endmodule
