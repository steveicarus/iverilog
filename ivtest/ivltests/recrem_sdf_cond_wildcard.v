// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module recrem_cond_wildcard(C,R,E);
 input C,R,E;
 reg nb,none,nzero,nplain,nedge;
 initial begin nb=0; none=0; nzero=0; nplain=0; nedge=0; end
 specify
  $recrem(posedge R &&& (E),posedge C,5,5,nb);
  $recrem(posedge R &&& (E === 1'b1),posedge C,5,5,none);
  $recrem(posedge R &&& (E === 1'b0),posedge C,5,5,nzero);
  $recrem(posedge R,posedge C,5,5,nplain);
  $recrem(posedge R &&& (E),negedge C,5,5,nedge);
 endspecify
endmodule
module tb;
 reg C,R,E,F;
 recrem_cond_wildcard u(C,R,E);
initial begin C=0;R=0;E=0;F=1'bx;
 #90 E=1;F=1; #10 R=1; #15 C=1; #1 R=0; #4 R=1; #30 C=0;R=0;
 #5 R=1; #25 R=0; #10 E=0;F=0;
 #10 R=1; #15 C=1; #1 R=0; #4 R=1; #30 C=0;R=0;
end
initial begin #1; $sdf_annotate("ivltests/recrem_sdf_cond_wildcard.sdf",tb);
$rrc_population("tb.u",5);
$rrc_watch("tb.u", "recrem", "R", "posedge", "C", "posedge", 20, 20, 8, 8, "nb", 1'b0, 2, 115, 120);
$rrc_watch("tb.u", "recrem", "R", "posedge", "C", "posedge", 20, 20, 8, 8, "none", 1'b0, 2, 115, 120);
$rrc_watch("tb.u", "recrem", "R", "posedge", "C", "posedge", 20, 20, 8, 8, "nzero", 1'b0, 2, 215, 220);
$rrc_watch("tb.u", "recrem", "R", "posedge", "C", "posedge", 20, 20, 8, 8, "nplain", 1'b0, 4, 115, 120, 215, 220);
$rrc_watch("tb.u", "recrem", "R", "posedge", "C", "negedge", 20, 20, 8, 8, "nedge", 1'b0, 1, 155);
$rrc_start; #279; $rrc_finish; $finish; end
endmodule
