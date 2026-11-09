// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`include "ivltests/setuphold_sdf_cell.vh"
module tb;reg C,D;
shc_sdf_cell negative_setup(C,D);
shc_sdf_cell negative_hold(C,D);
shc_sdf_cell overflow(C,D);
shc_sdf_cell valid(C,D);
initial begin C=0;D=0;#100 D=1;#15 C=1;#5 D=0;end
initial begin #1;
$sdf_annotate("ivltests/setuphold_hdl_reject.sdf",tb);
$shc_population("tb.negative_setup", 1);
$shc_population("tb.negative_hold", 1);
$shc_population("tb.overflow", 1);
$shc_population("tb.valid", 1);
$shc_watch("tb.negative_setup", "setuphold", "C", "posedge", "D", "any", 10, 10, 6, 6, "notifier", 1'b0, 1, 120);
$shc_watch("tb.negative_hold", "setuphold", "C", "posedge", "D", "any", 10, 10, 6, 6, "notifier", 1'b0, 1, 120);
$shc_watch("tb.overflow", "setuphold", "C", "posedge", "D", "any", 10, 10, 6, 6, "notifier", 1'b0, 1, 120);
$shc_watch("tb.valid", "setuphold", "C", "posedge", "D", "any", 20, 20, 8, 8, "notifier", 1'b0, 2, 115, 120);
$shc_start; #149; $shc_finish; $finish; end
endmodule
