// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
// All rejected entries would change limit 10 to 30 and add callbacks at 115/130.
// A valid following CELL must bind 20 and produce both episodes, proving parser continuation.
`timescale 1ps/1ps
module dut(R,C,E,F);
  input R,C,E,F;
  reg ns,nh; initial begin ns=0;nh=0;end
  specify
    $recovery(R &&& E, posedge C &&& F === 1'b0, 10, ns);
    $removal(R &&& E, posedge C &&& F === 1'b0, 10, nh);
  endspecify
endmodule

module tb;
  reg C,R,E,F; dut u(R,C,E,F);
  dut valid(R,C,E,F);
  initial begin C=0;R=0;E=1;F=0;
    #100 R=1;#15 C=1;#15 R=0;#10 C=0;#60 R=1;#5 C=1;#3 R=0;
  end
  initial begin
    #1;
    $sdf_annotate("ivltests/recovery_removal_sdf_cond_reject.sdf",tb);
    $rr_watch("tb.u", "recovery", "R", "any", "C", "posedge", 10, 10, "ns", 1, 205);
    $rr_watch("tb.u", "removal", "R", "any", "C", "posedge", 10, 10, "nh", 1, 208);
    $rr_watch("tb.valid", "recovery", "R", "any", "C", "posedge", 20, 20, "ns", 2, 115, 205);
    $rr_watch("tb.valid", "removal", "R", "any", "C", "posedge", 20, 20, "nh", 2, 130, 208);
    $tchk_condition("tb.u", "ns", "reference", "bare", "E", 0);
    $tchk_condition("tb.u", "ns", "data", "case", "F", 0);
    $tchk_condition("tb.u", "nh", "reference", "bare", "E", 0);
    $tchk_condition("tb.u", "nh", "data", "case", "F", 0);
    $tchk_condition("tb.valid", "ns", "reference", "bare", "E", 0);
    $tchk_condition("tb.valid", "ns", "data", "case", "F", 0);
    $tchk_condition("tb.valid", "nh", "reference", "bare", "E", 0);
    $tchk_condition("tb.valid", "nh", "data", "case", "F", 0);
    $rr_start;
    #229; $rr_finish; $finish;
  end
endmodule
