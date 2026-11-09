// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`include "ivltests/setuphold_cells.vh"
module tb;reg C,D;shc_any a(C,D),b(C,D),neg_setup(C,D),neg_hold(C,D),overflow(C,D);
 initial begin C=0;D=0;#100 D=1;#15 C=1;#5 D=0;end
initial begin #1;
$shc_put("tb.a",20,8,"real");
$shc_put("tb.b",20,8,"ticks");
$shc_put("tb.neg_setup",-1,99,"real");
$shc_put("tb.neg_hold",99,-1,"real");
$shc_put("tb.overflow",99,1e25,"real");
$shc_population("tb.a", 1);
$shc_population("tb.b", 1);
$shc_population("tb.neg_setup", 1);
$shc_population("tb.neg_hold", 1);
$shc_population("tb.overflow", 1);
$shc_watch("tb.a", "setuphold", "C", "posedge", "D", "any", 20, 20, 8, 8, "notifier", 1'b0, 2, 115, 120);
$shc_watch("tb.b", "setuphold", "C", "posedge", "D", "any", 20, 20, 8, 8, "notifier", 1'b0, 2, 115, 120);
$shc_watch("tb.neg_setup", "setuphold", "C", "posedge", "D", "any", 10, 10, 6, 6, "notifier", 1'b0, 1, 120);
$shc_watch("tb.neg_hold", "setuphold", "C", "posedge", "D", "any", 10, 10, 6, 6, "notifier", 1'b0, 1, 120);
$shc_watch("tb.overflow", "setuphold", "C", "posedge", "D", "any", 10, 10, 6, 6, "notifier", 1'b0, 1, 120);
$shc_start; #149; $shc_finish; $finish; end
endmodule
