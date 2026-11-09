// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`include "ivltests/recrem_cells.vh"
module tb;reg cz,dz,cr,dr,cn,dn;
 rrc_pos #(10,10) data0(cz,dz),ref0(cr,dr),missing(cn,dn);
 // In data0 only the control/reference edge occurs at time zero;
 // in ref0 only the clock/data edge occurs. A missing history is distinct from t=0.
 initial begin cz=0;dz=1;#9 cz=1;#1 cz=0;#10 cz=1;end
 initial begin cr=1;dr=0;#9 dr=1;end
 initial begin cn=0;#20 cn=1;#1 cn=0;#79 dn=1;#9 cn=1;end
initial begin #1;

$rrc_population("tb.data0", 1);
$rrc_population("tb.ref0", 1);
$rrc_population("tb.missing", 1);
$rrc_watch("tb.data0", "recrem", "D", "posedge", "C", "posedge", 10, 10, 10, 10, "notifier", 1, 9);
$rrc_watch("tb.ref0", "recrem", "D", "posedge", "C", "posedge", 10, 10, 10, 10, "notifier", 1, 9);
$rrc_watch("tb.missing", "recrem", "D", "posedge", "C", "posedge", 10, 10, 10, 10, "notifier", 1, 109);
$rrc_start; #139; $rrc_finish; $finish; end
endmodule
