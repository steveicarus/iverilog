// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module setuphold_cond_cross(C,D,E,F);
 input C,D,E,F;
 reg nfirst,nsecond,ncombined; initial begin nfirst=0;nsecond=0;ncombined=0;end
 specify
  $setup(D &&& (F === 1'b0),posedge C &&& (E),5,nfirst);
  $hold(posedge C &&& (E),D &&& (F === 1'b0),5,nsecond);
  $setuphold(posedge C &&& (E),D &&& (F === 1'b0),5,5,ncombined);
 endspecify
endmodule
module tb;
 reg C,D,E,F;
 setuphold_cond_cross combined(C,D,E,F);
 setuphold_cond_cross separate(C,D,E,F);
 setuphold_cond_cross first(C,D,E,F);
 setuphold_cond_cross second(C,D,E,F);
initial begin C=0;D=0;E=0;F=1'bx; #90 E=1;F=0; #10 D=1; #15 C=1; #5 D=0; end
initial begin #1; $sdf_annotate("ivltests/setuphold_sdf_cond_cross.sdf",tb);
$shc_population("tb.combined",3);
$shc_watch("tb.combined", "setup", "C", "posedge", "D", "any", 20, 20, "nfirst", 1'b0, 1, 115);
$shc_watch("tb.combined", "hold", "C", "posedge", "D", "any", 8, 8, "nsecond", 1'b0, 1, 120);
$shc_watch("tb.combined", "setuphold", "C", "posedge", "D", "any", 20, 20, 8, 8, "ncombined", 1'b0, 2, 115, 120);
$shc_population("tb.separate",3);
$shc_watch("tb.separate", "setup", "C", "posedge", "D", "any", 20, 20, "nfirst", 1'b0, 1, 115);
$shc_watch("tb.separate", "hold", "C", "posedge", "D", "any", 8, 8, "nsecond", 1'b0, 1, 120);
$shc_watch("tb.separate", "setuphold", "C", "posedge", "D", "any", 20, 20, 8, 8, "ncombined", 1'b0, 2, 115, 120);
$shc_population("tb.first",3);
$shc_watch("tb.first", "setup", "C", "posedge", "D", "any", 20, 20, "nfirst", 1'b0, 1, 115);
$shc_watch("tb.first", "hold", "C", "posedge", "D", "any", 5, 5, "nsecond", 1'b0, 0);
$shc_watch("tb.first", "setuphold", "C", "posedge", "D", "any", 20, 20, 5, 5, "ncombined", 1'b0, 1, 115);
$shc_population("tb.second",3);
$shc_watch("tb.second", "setup", "C", "posedge", "D", "any", 5, 5, "nfirst", 1'b0, 0);
$shc_watch("tb.second", "hold", "C", "posedge", "D", "any", 8, 8, "nsecond", 1'b0, 1, 120);
$shc_watch("tb.second", "setuphold", "C", "posedge", "D", "any", 5, 5, 8, 8, "ncombined", 1'b0, 1, 120);
$shc_start; #159; $shc_finish; $finish; end
endmodule
