// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`include "ivltests/recrem_sdf_cell.vh"
module tb;reg C,D;
rrc_sdf_cell negative_recovery(C,D);
rrc_sdf_cell negative_removal(C,D);
rrc_sdf_cell overflow(C,D);
rrc_sdf_cell valid(C,D);
initial begin C=0;D=0;#100 D=1;#15 C=1;#5 D=0;end
initial begin #1;
$sdf_annotate("ivltests/recrem_hdl_reject.sdf",tb);
$rrc_population("tb.negative_recovery", 1);
$rrc_population("tb.negative_removal", 1);
$rrc_population("tb.overflow", 1);
$rrc_population("tb.valid", 1);
$rrc_watch("tb.negative_recovery", "recrem", "D", "any", "C", "posedge", 10, 10, 6, 6, "notifier", 1'b0, 1, 120);
$rrc_watch("tb.negative_removal", "recrem", "D", "any", "C", "posedge", 10, 10, 6, 6, "notifier", 1'b0, 1, 120);
$rrc_watch("tb.overflow", "recrem", "D", "any", "C", "posedge", 10, 10, 6, 6, "notifier", 1'b0, 1, 120);
$rrc_watch("tb.valid", "recrem", "D", "any", "C", "posedge", 20, 20, 8, 8, "notifier", 1'b0, 2, 115, 120);
$rrc_start; #149; $rrc_finish; $finish; end
endmodule
