// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`include "ivltests/setuphold_cells.vh"
module shc_alias(A);parameter S=10,H=6;input A;reg notifier;initial notifier=0;
 specify $setuphold(posedge A,posedge A,S,H,notifier);endspecify endmodule
module tb;reg A;shc_pos twoports(A,A);shc_alias p(A);shc_alias #(0,6) h(A);shc_alias #(10,0) s(A);shc_alias #(0,0) z(A);
 initial begin A=0;#100 A=1;#20 A=0;#80 A=1;end
initial begin #1;

$shc_population("tb.twoports", 1);
$shc_population("tb.p", 1);
$shc_population("tb.h", 1);
$shc_population("tb.s", 1);
$shc_population("tb.z", 1);
$shc_watch("tb.p", "setuphold", "A", "posedge", "A", "posedge", 10, 10, 6, 6, "notifier", 1'b0, 2, 100, 200);
$shc_watch("tb.h", "setuphold", "A", "posedge", "A", "posedge", 0, 0, 6, 6, "notifier", 1'b0, 2, 100, 200);
$shc_watch("tb.s", "setuphold", "A", "posedge", "A", "posedge", 10, 10, 0, 0, "notifier", 1'b0, 0);
$shc_watch("tb.z", "setuphold", "A", "posedge", "A", "posedge", 0, 0, 0, 0, "notifier", 1'b0, 0);
$shc_watch("tb.twoports", "setuphold", "C", "posedge", "D", "posedge", 10, 10, 6, 6, "notifier", 1'b0, 2, 100, 200);
$shc_start; #249; $shc_finish; $finish; end
endmodule
