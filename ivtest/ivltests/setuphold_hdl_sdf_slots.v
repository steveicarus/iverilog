// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`include "ivltests/setuphold_sdf_cell.vh"
module tb;reg C,D;shc_sdf_cell u(C,D);
 initial begin C=0;D=0;#100 D=1;#15 C=1;#5 D=0;end
 initial begin #1;$sdf_annotate("ivltests/setuphold_hdl_slots.sdf",tb);$shc_population("tb.u",1);
`ifdef SHC_MIN
$shc_watch("tb.u", "setuphold", "C", "posedge", "D", "any", 10, 10, 3, 3, "notifier", 1'b0, 0);
`elsif SHC_MAX
$shc_watch("tb.u", "setuphold", "C", "posedge", "D", "any", 30, 30, 13, 13, "notifier", 1'b0, 2, 115, 120);
`else
$shc_watch("tb.u", "setuphold", "C", "posedge", "D", "any", 20, 20, 8, 8, "notifier", 1'b0, 2, 115, 120);
`endif
 $shc_start;#149;$shc_finish;$finish;end endmodule
