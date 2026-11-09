// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module renamed(input CLK,RESET,RESET_OK,CLOCK_OK);reg notifier;initial notifier=0;
 specify $recrem(posedge RESET &&& RESET_OK,posedge CLK &&& (CLOCK_OK === 1'b1),10,6,notifier);endspecify endmodule
module tb;
 reg C,R,RE,CE;renamed second(.CLOCK_OK(CE),.RESET(R),.CLK(C),.RESET_OK(RE));renamed first(C,R,RE,CE);
 initial begin C=0;R=0;RE=1;CE=1;#100 R=1;#5 C=1;#15 R=0;C=0;#80 C=1;#5 R=1;end
 initial begin #1;
$rrc_population("tb.second", 1);
$rrc_watch("tb.second", "recrem", "RESET", "posedge", "CLK", "posedge", 10, 10, 6, 6, "notifier", 1'b0, 2, 105, 205);
$rrc_population("tb.first", 1);
$rrc_watch("tb.first", "recrem", "RESET", "posedge", "CLK", "posedge", 10, 10, 6, 6, "notifier", 1'b0, 2, 105, 205);
 $rrc_start; #249; $rrc_finish; $finish; end
endmodule
