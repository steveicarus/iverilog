// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module recrem_cond_cross(C,R,E,F);
 input C,R,E,F;
 reg nfirst,nsecond,ncombined; initial begin nfirst=0;nsecond=0;ncombined=0;end
 specify
  $recovery(posedge R &&& (E),posedge C &&& (F === 1'b0),5,nfirst);
  $removal(posedge R &&& (E),posedge C &&& (F === 1'b0),5,nsecond);
  $recrem(posedge R &&& (E),posedge C &&& (F === 1'b0),5,5,ncombined);
 endspecify
endmodule
module tb;
 reg C,R,E,F;
 recrem_cond_cross combined(C,R,E,F);
 recrem_cond_cross separate(C,R,E,F);
 recrem_cond_cross first(C,R,E,F);
 recrem_cond_cross second(C,R,E,F);
initial begin C=0;R=0;E=0;F=1'bx; #90 E=1;F=0; #10 R=1; #15 C=1; #1 R=0; #4 R=1; end
initial begin #1; $sdf_annotate("ivltests/recrem_sdf_cond_cross.sdf",tb);
$rrc_population("tb.combined",3);
$rrc_watch("tb.combined", "recovery", "R", "posedge", "C", "posedge", 20, 20, "nfirst", 1'b0, 1, 115);
$rrc_watch("tb.combined", "removal", "R", "posedge", "C", "posedge", 8, 8, "nsecond", 1'b0, 1, 120);
$rrc_watch("tb.combined", "recrem", "R", "posedge", "C", "posedge", 20, 20, 8, 8, "ncombined", 1'b0, 2, 115, 120);
$rrc_population("tb.separate",3);
$rrc_watch("tb.separate", "recovery", "R", "posedge", "C", "posedge", 20, 20, "nfirst", 1'b0, 1, 115);
$rrc_watch("tb.separate", "removal", "R", "posedge", "C", "posedge", 8, 8, "nsecond", 1'b0, 1, 120);
$rrc_watch("tb.separate", "recrem", "R", "posedge", "C", "posedge", 20, 20, 8, 8, "ncombined", 1'b0, 2, 115, 120);
$rrc_population("tb.first",3);
$rrc_watch("tb.first", "recovery", "R", "posedge", "C", "posedge", 20, 20, "nfirst", 1'b0, 1, 115);
$rrc_watch("tb.first", "removal", "R", "posedge", "C", "posedge", 5, 5, "nsecond", 1'b0, 0);
$rrc_watch("tb.first", "recrem", "R", "posedge", "C", "posedge", 20, 20, 5, 5, "ncombined", 1'b0, 1, 115);
$rrc_population("tb.second",3);
$rrc_watch("tb.second", "recovery", "R", "posedge", "C", "posedge", 5, 5, "nfirst", 1'b0, 0);
$rrc_watch("tb.second", "removal", "R", "posedge", "C", "posedge", 8, 8, "nsecond", 1'b0, 1, 120);
$rrc_watch("tb.second", "recrem", "R", "posedge", "C", "posedge", 5, 5, 8, 8, "ncombined", 1'b0, 1, 120);
$rrc_start; #159; $rrc_finish; $finish; end
endmodule
