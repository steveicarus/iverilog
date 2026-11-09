// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ns/10ps
module rrc_sdf_units(C,D);input C,D;reg notifier;initial notifier=0;
 specify $recrem(D,posedge C,1,1,notifier);endspecify endmodule
`timescale 1ps/1ps
module tb;reg C,D;rrc_sdf_units u(C,D);
 initial begin C=0;D=0;#100 D=1;#19 C=1;#29 D=0;#2 D=1;end
initial begin #1;
$sdf_annotate("ivltests/recrem_hdl_units.sdf",tb);
$rrc_population("tb.u", 1);
$rrc_watch("tb.u", "recrem", "D", "any", "C", "posedge", 0.02, 20, 0.03, 30, "notifier", 2, 119, 148);
$rrc_start; #199; $rrc_finish; $finish; end
endmodule
