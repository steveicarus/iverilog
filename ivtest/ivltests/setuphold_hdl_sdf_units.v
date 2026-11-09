// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ns/10ps
module shc_sdf_units(C,D);input C,D;reg notifier;initial notifier=0;
 specify $setuphold(posedge C,D,1,1,notifier);endspecify endmodule
`timescale 1ps/1ps
module tb;reg C,D;shc_sdf_units u(C,D);
 initial begin C=0;D=0;#100 D=1;#19 C=1;#29 D=0;#2 D=1;end
initial begin #1;
$sdf_annotate("ivltests/setuphold_hdl_units.sdf",tb);
$shc_population("tb.u", 1);
$shc_watch("tb.u", "setuphold", "C", "posedge", "D", "any", 0.02, 20, 0.03, 30, "notifier", 1'b0, 2, 119, 148);
$shc_start; #199; $shc_finish; $finish; end
endmodule
