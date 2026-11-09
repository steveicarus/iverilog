// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module setuphold_cond_select(C,D,D2,C2,D3,E,F);
 input C,D,D2,C2,D3,E,F;
 reg nrb,nr1,nr0,ndb,nd1,nd0,nboth,nplain,nedge;
 initial begin nrb=0; nr1=0; nr0=0; ndb=0; nd1=0; nd0=0; nboth=0; nplain=0; nedge=0; end
 specify
  $setuphold(posedge C &&& (E),D,5,5,nrb);
  $setuphold(posedge C &&& (E === 1'b1),D,5,5,nr1);
  $setuphold(posedge C &&& (E === 1'b0),D,5,5,nr0);
  $setuphold(posedge C,D2 &&& (E),5,5,ndb);
  $setuphold(posedge C,D2 &&& (E === 1'b1),5,5,nd1);
  $setuphold(posedge C,D2 &&& (E === 1'b0),5,5,nd0);
  $setuphold(posedge C2 &&& (E),D3 &&& (F),5,5,nboth);
  $setuphold(posedge C,D,5,5,nplain);
  $setuphold(negedge C &&& (E),D,5,5,nedge);
 endspecify
endmodule
module tb;
 reg C,D,E,F;
 setuphold_cond_select u(C,D,D,C,D,E,F);
initial begin C=0;D=0;E=0;F=1'bx;
 #90 E=1;F=1; #10 D=1; #15 C=1; #5 D=0; #30 C=0;
 #40 E=0;F=0;
 #10 D=1; #15 C=1; #5 D=0; #30 C=0;
end
initial begin #1;
`ifdef VLOG95_SDF
// Tied input ports specialize the translated module names.
$sdf_annotate("ivltests/setuphold_sdf_cond_select_vlog95.sdf",tb);
`else
$sdf_annotate("ivltests/setuphold_sdf_cond_select.sdf",tb);
`endif
$shc_population("tb.u",9);
$shc_watch("tb.u", "setuphold", "C", "posedge", "D", "any", 20, 20, 8, 8, "nrb", 1'b0, 2, 115, 120);
$shc_watch("tb.u", "setuphold", "C", "posedge", "D", "any", 12, 12, 4, 4, "nr1", 1'b0, 0);
$shc_watch("tb.u", "setuphold", "C", "posedge", "D", "any", 30, 30, 9, 9, "nr0", 1'b0, 2, 215, 220);
$shc_watch("tb.u", "setuphold", "C", "posedge", "D2", "any", 20, 20, 8, 8, "ndb", 1'b0, 2, 115, 120);
$shc_watch("tb.u", "setuphold", "C", "posedge", "D2", "any", 12, 12, 4, 4, "nd1", 1'b0, 0);
$shc_watch("tb.u", "setuphold", "C", "posedge", "D2", "any", 30, 30, 9, 9, "nd0", 1'b0, 2, 215, 220);
$shc_watch("tb.u", "setuphold", "C2", "posedge", "D3", "any", 20, 20, 8, 8, "nboth", 1'b0, 2, 115, 120);
$shc_watch("tb.u", "setuphold", "C", "posedge", "D", "any", 5, 5, 5, 5, "nplain", 1'b0, 0);
$shc_watch("tb.u", "setuphold", "C", "negedge", "D", "any", 5, 5, 5, 5, "nedge", 1'b0, 0);
$tchk_condition("tb.u", "nrb", "reference", "bare", "E", 0);
$tchk_condition("tb.u", "nrb", "data", "none", "", 0);
$tchk_condition("tb.u", "nr1", "reference", "case", "E", 1);
$tchk_condition("tb.u", "nr1", "data", "none", "", 0);
$tchk_condition("tb.u", "nr0", "reference", "case", "E", 0);
$tchk_condition("tb.u", "nr0", "data", "none", "", 0);
$tchk_condition("tb.u", "ndb", "reference", "none", "", 0);
$tchk_condition("tb.u", "ndb", "data", "bare", "E", 0);
$tchk_condition("tb.u", "nd1", "reference", "none", "", 0);
$tchk_condition("tb.u", "nd1", "data", "case", "E", 1);
$tchk_condition("tb.u", "nd0", "reference", "none", "", 0);
$tchk_condition("tb.u", "nd0", "data", "case", "E", 0);
$tchk_condition("tb.u", "nboth", "reference", "bare", "E", 0);
$tchk_condition("tb.u", "nboth", "data", "bare", "F", 0);
$tchk_condition("tb.u", "nplain", "reference", "none", "", 0);
$tchk_condition("tb.u", "nplain", "data", "none", "", 0);
$tchk_condition("tb.u", "nedge", "reference", "bare", "E", 0);
$tchk_condition("tb.u", "nedge", "data", "none", "", 0);
$shc_start; #90;
$tchk_condition("tb.u", "nrb", "reference", "bare", "E", 0);
$tchk_condition("tb.u", "nrb", "data", "none", "", 0);
$tchk_condition("tb.u", "nr1", "reference", "case", "E", 1);
$tchk_condition("tb.u", "nr1", "data", "none", "", 0);
$tchk_condition("tb.u", "nr0", "reference", "case", "E", 0);
$tchk_condition("tb.u", "nr0", "data", "none", "", 0);
$tchk_condition("tb.u", "ndb", "reference", "none", "", 0);
$tchk_condition("tb.u", "ndb", "data", "bare", "E", 0);
$tchk_condition("tb.u", "nd1", "reference", "none", "", 0);
$tchk_condition("tb.u", "nd1", "data", "case", "E", 1);
$tchk_condition("tb.u", "nd0", "reference", "none", "", 0);
$tchk_condition("tb.u", "nd0", "data", "case", "E", 0);
$tchk_condition("tb.u", "nboth", "reference", "bare", "E", 0);
$tchk_condition("tb.u", "nboth", "data", "bare", "F", 0);
$tchk_condition("tb.u", "nplain", "reference", "none", "", 0);
$tchk_condition("tb.u", "nplain", "data", "none", "", 0);
$tchk_condition("tb.u", "nedge", "reference", "bare", "E", 0);
$tchk_condition("tb.u", "nedge", "data", "none", "", 0);
#189; $shc_finish; $finish; end
endmodule
