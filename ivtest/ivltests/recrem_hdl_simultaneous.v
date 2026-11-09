// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`include "ivltests/recrem_cells.vh"
module tb; reg C,D; rrc_pos u(C,D);
 initial begin C=0;D=0;
 #100 D=1;#0 C=1;
 #20 C=0;D=0;
 #80 C=1;#0 D=1;
 end
initial begin #1;

$rrc_population("tb.u", 1);
$rrc_watch("tb.u", "recrem", "D", "posedge", "C", "posedge", 10, 10, 6, 6, "notifier", 1'b0, 2, 100, 200);
$rrc_start; #249; $rrc_finish; $finish; end
endmodule
