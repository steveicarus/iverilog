// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module setuphold_cond_wildcard(C,D,E);
 input C,D,E;
 reg nb,none,nzero,nplain,nedge;
 initial begin nb=0; none=0; nzero=0; nplain=0; nedge=0; end
 specify
  $setuphold(posedge C &&& (E),D,5,5,nb);
  $setuphold(posedge C &&& (E === 1'b1),D,5,5,none);
  $setuphold(posedge C &&& (E === 1'b0),D,5,5,nzero);
  $setuphold(posedge C,D,5,5,nplain);
  $setuphold(negedge C &&& (E),D,5,5,nedge);
 endspecify
endmodule
module tb;
 reg C,D,E,F;
 setuphold_cond_wildcard u(C,D,E);
initial begin C=0;D=0;E=0;F=1'bx;
 #90 E=1;F=1; #10 D=1; #15 C=1; #5 D=0; #30 C=0;
 #5 D=1; #35 E=0;F=0;
 #10 D=0; #15 C=1; #5 D=1; #30 C=0;
end
initial begin #1; $sdf_annotate("ivltests/setuphold_sdf_cond_wildcard.sdf",tb);
$shc_population("tb.u",5);
$shc_watch("tb.u", "setuphold", "C", "posedge", "D", "any", 20, 20, 8, 8, "nb", 1'b0, 2, 115, 120);
$shc_watch("tb.u", "setuphold", "C", "posedge", "D", "any", 20, 20, 8, 8, "none", 1'b0, 2, 115, 120);
$shc_watch("tb.u", "setuphold", "C", "posedge", "D", "any", 20, 20, 8, 8, "nzero", 1'b0, 2, 215, 220);
$shc_watch("tb.u", "setuphold", "C", "posedge", "D", "any", 20, 20, 8, 8, "nplain", 1'b0, 4, 115, 120, 215, 220);
$shc_watch("tb.u", "setuphold", "C", "negedge", "D", "any", 20, 20, 8, 8, "nedge", 1'b0, 1, 155);
$shc_start; #279; $shc_finish; $finish; end
endmodule
