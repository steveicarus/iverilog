// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module recrem_cond_select(C,R,R2,R3,C2,E,F);
 input C,R,R2,R3,C2,E,F;
 reg nrb,nr1,nr0,ndb,nd1,nd0,nboth,nplain,nedge;
 initial begin nrb=0; nr1=0; nr0=0; ndb=0; nd1=0; nd0=0; nboth=0; nplain=0; nedge=0; end
 specify
  $recrem(posedge R &&& (E),posedge C,5,5,nrb);
  $recrem(posedge R &&& (E === 1'b1),posedge C,5,5,nr1);
  $recrem(posedge R &&& (E === 1'b0),posedge C,5,5,nr0);
  $recrem(posedge R2,posedge C &&& (E),5,5,ndb);
  $recrem(posedge R2,posedge C &&& (E === 1'b1),5,5,nd1);
  $recrem(posedge R2,posedge C &&& (E === 1'b0),5,5,nd0);
  $recrem(posedge R3 &&& (E),posedge C2 &&& (F),5,5,nboth);
  $recrem(posedge R,posedge C,5,5,nplain);
  $recrem(posedge R &&& (E),negedge C,5,5,nedge);
 endspecify
endmodule
module tb;
 reg C,R,E,F;
 recrem_cond_select u(C,R,R,R,C,E,F);
initial begin C=0;R=0;E=0;F=1'bx;
 #90 E=1;F=1; #10 R=1; #15 C=1; #1 R=0; #4 R=1; #30 C=0;R=0;
 #40 E=0;F=0;
 #10 R=1; #15 C=1; #1 R=0; #4 R=1; #30 C=0;R=0;
end
initial begin #1;
`ifdef VLOG95_SDF
// Tied input ports specialize the translated module names.
$sdf_annotate("ivltests/recrem_sdf_cond_select_vlog95.sdf",tb);
`else
$sdf_annotate("ivltests/recrem_sdf_cond_select.sdf",tb);
`endif
$rrc_population("tb.u",9);
$rrc_watch("tb.u", "recrem", "R", "posedge", "C", "posedge", 20, 20, 8, 8, "nrb", 1'b0, 2, 115, 120);
$rrc_watch("tb.u", "recrem", "R", "posedge", "C", "posedge", 12, 12, 4, 4, "nr1", 1'b0, 0);
$rrc_watch("tb.u", "recrem", "R", "posedge", "C", "posedge", 30, 30, 9, 9, "nr0", 1'b0, 2, 215, 220);
$rrc_watch("tb.u", "recrem", "R2", "posedge", "C", "posedge", 20, 20, 8, 8, "ndb", 1'b0, 2, 115, 120);
$rrc_watch("tb.u", "recrem", "R2", "posedge", "C", "posedge", 12, 12, 4, 4, "nd1", 1'b0, 0);
$rrc_watch("tb.u", "recrem", "R2", "posedge", "C", "posedge", 30, 30, 9, 9, "nd0", 1'b0, 2, 215, 220);
$rrc_watch("tb.u", "recrem", "R3", "posedge", "C2", "posedge", 20, 20, 8, 8, "nboth", 1'b0, 2, 115, 120);
$rrc_watch("tb.u", "recrem", "R", "posedge", "C", "posedge", 5, 5, 5, 5, "nplain", 1'b0, 0);
$rrc_watch("tb.u", "recrem", "R", "posedge", "C", "negedge", 5, 5, 5, 5, "nedge", 1'b0, 0);
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
$rrc_start; #90;
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
#189; $rrc_finish; $finish; end
endmodule
