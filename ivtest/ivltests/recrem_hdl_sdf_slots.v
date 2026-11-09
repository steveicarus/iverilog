// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`include "ivltests/recrem_sdf_cell.vh"
module tb;reg C,D;rrc_sdf_cell u(C,D);
 initial begin C=0;D=0;#100 D=1;#15 C=1;#5 D=0;end
 initial begin #1;$sdf_annotate("ivltests/recrem_hdl_slots.sdf",tb);$rrc_population("tb.u",1);
`ifdef RRC_MIN
$rrc_watch("tb.u", "recrem", "D", "any", "C", "posedge", 10, 10, 3, 3, "notifier", 1'b0, 0);
`elsif RRC_MAX
$rrc_watch("tb.u", "recrem", "D", "any", "C", "posedge", 30, 30, 13, 13, "notifier", 1'b0, 2, 115, 120);
`else
$rrc_watch("tb.u", "recrem", "D", "any", "C", "posedge", 20, 20, 8, 8, "notifier", 1'b0, 2, 115, 120);
`endif
 $rrc_start;#149;$rrc_finish;$finish;end endmodule
