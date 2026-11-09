// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`include "ivltests/recrem_cells.vh"
module tb; reg C,D; rrc_pos u(C,D);
 initial begin C=0;D=0;
 #100 D=1;#9 C=1;#11 C=0;D=0;
 #80 D=1;#10 C=1;#10 C=0;D=0;
 #80 D=1;#11 C=1;#9 C=0;D=0;
 #80 C=1;#5 D=1;#15 C=0;D=0;
 #80 C=1;#6 D=1;#14 C=0;D=0;
 #80 C=1;#7 D=1;
 end
initial begin #1;

$rrc_population("tb.u", 1);
$rrc_watch("tb.u", "recrem", "D", "posedge", "C", "posedge", 10, 10, 6, 6, "notifier", 2, 109, 405);
$rrc_start; #699; $rrc_finish; $finish; end
endmodule
