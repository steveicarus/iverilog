// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`include "ivltests/setuphold_cells.vh"
module tb;reg cz,dz,cr,dr,cn,dn;
 shc_pos #(10,10) data0(cz,dz),ref0(cr,dr),missing(cn,dn);
 // Only the data edge occurs at time zero in data0; only reference in ref0.
 initial begin cz=0;dz=1;#9 cz=1;#1 cz=0;#10 cz=1;end
 initial begin cr=1;dr=0;#9 dr=1;end
 initial begin cn=0;#20 cn=1;#1 cn=0;#79 dn=1;#9 cn=1;end
initial begin #1;

$shc_population("tb.data0", 1);
$shc_population("tb.ref0", 1);
$shc_population("tb.missing", 1);
$shc_watch("tb.data0", "setuphold", "C", "posedge", "D", "posedge", 10, 10, 10, 10, "notifier", 1'b0, 1, 9);
$shc_watch("tb.ref0", "setuphold", "C", "posedge", "D", "posedge", 10, 10, 10, 10, "notifier", 1'b0, 1, 9);
$shc_watch("tb.missing", "setuphold", "C", "posedge", "D", "posedge", 10, 10, 10, 10, "notifier", 1'b0, 1, 109);
$shc_start; #139; $shc_finish; $finish; end
endmodule
