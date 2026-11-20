// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`include "ivltests/setuphold_cells.vh"
module tb; reg C,D; shc_pos u(C,D);
 initial begin C=0;D=0;
 #100 D=1;#0 C=1;
 #20 C=0;D=0;
 #80 C=1;#0 D=1;
 end
initial begin #1;

$shc_population("tb.u", 1);
$shc_watch("tb.u", "setuphold", "C", "posedge", "D", "posedge", 10, 10, 6, 6, "notifier", 1'b0, 2, 100, 200);
$shc_start; #249; $shc_finish; $finish; end
endmodule
