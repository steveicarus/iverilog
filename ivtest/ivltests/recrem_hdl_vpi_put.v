// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`include "ivltests/recrem_cells.vh"
module tb;reg C,D;rrc_any a(C,D),b(C,D),neg_recovery(C,D),neg_removal(C,D),overflow(C,D);
 initial begin C=0;D=0;#100 D=1;#15 C=1;#5 D=0;end
initial begin #1;
$rrc_put("tb.a",20,8,"real");
$rrc_put("tb.b",20,8,"ticks");
$rrc_put("tb.neg_recovery",-1,99,"real");
$rrc_put("tb.neg_removal",99,-1,"real");
$rrc_put("tb.overflow",99,1e25,"real");
$rrc_population("tb.a", 1);
$rrc_population("tb.b", 1);
$rrc_population("tb.neg_recovery", 1);
$rrc_population("tb.neg_removal", 1);
$rrc_population("tb.overflow", 1);
$rrc_watch("tb.a", "recrem", "D", "any", "C", "posedge", 20, 20, 8, 8, "notifier", 2, 115, 120);
$rrc_watch("tb.b", "recrem", "D", "any", "C", "posedge", 20, 20, 8, 8, "notifier", 2, 115, 120);
$rrc_watch("tb.neg_recovery", "recrem", "D", "any", "C", "posedge", 10, 10, 6, 6, "notifier", 1, 120);
$rrc_watch("tb.neg_removal", "recrem", "D", "any", "C", "posedge", 10, 10, 6, 6, "notifier", 1, 120);
$rrc_watch("tb.overflow", "recrem", "D", "any", "C", "posedge", 10, 10, 6, 6, "notifier", 1, 120);
$rrc_start; #149; $rrc_finish; $finish; end
endmodule
