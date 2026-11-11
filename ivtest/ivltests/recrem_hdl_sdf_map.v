// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module rrc_map(C,D,E);input C,D,E;reg ns,nh,nc;initial begin ns=0;nh=0;nc=0;end
 specify $recovery(D,posedge C,5,ns);$removal(D,posedge C,6,nh);$recrem(D,posedge C,5,6,nc);endspecify
endmodule
module tb;reg C,D,E;
rrc_map empty_recovery(C,D,E),empty_removal(C,D,E);
rrc_map combined(C,D,E);
rrc_map partial_recovery(C,D,E);
rrc_map partial_removal(C,D,E);
rrc_map separate(C,D,E);
rrc_map wrong_edge(C,D,E);
rrc_map wrong_data(C,D,E);
initial begin C=0;D=0;E=0;#100 D=1;#15 C=1;#5 D=0;end
initial begin #1;
$sdf_annotate("ivltests/recrem_hdl_map.sdf",tb);
$rrc_population("tb.empty_recovery", 3);
$rrc_population("tb.empty_removal", 3);
$rrc_population("tb.combined", 3);
$rrc_population("tb.partial_recovery", 3);
$rrc_population("tb.partial_removal", 3);
$rrc_population("tb.separate", 3);
$rrc_population("tb.wrong_edge", 3);
$rrc_population("tb.wrong_data", 3);
$rrc_watch("tb.combined", "recovery", "D", "any", "C", "posedge", 20, 20, "ns", 1'b0, 1, 115);
$rrc_watch("tb.combined", "removal", "D", "any", "C", "posedge", 8, 8, "nh", 1'b0, 1, 120);
$rrc_watch("tb.combined", "recrem", "D", "any", "C", "posedge", 20, 20, 8, 8, "nc", 1'b0, 2, 115, 120);
$rrc_watch("tb.partial_recovery", "recovery", "D", "any", "C", "posedge", 7, 7, "ns", 1'b0, 0);
$rrc_watch("tb.partial_recovery", "removal", "D", "any", "C", "posedge", 8, 8, "nh", 1'b0, 1, 120);
$rrc_watch("tb.partial_recovery", "recrem", "D", "any", "C", "posedge", 7, 7, 8, 8, "nc", 1'b0, 1, 120);
$rrc_watch("tb.partial_removal", "recovery", "D", "any", "C", "posedge", 20, 20, "ns", 1'b0, 1, 115);
$rrc_watch("tb.partial_removal", "removal", "D", "any", "C", "posedge", 3, 3, "nh", 1'b0, 0);
$rrc_watch("tb.partial_removal", "recrem", "D", "any", "C", "posedge", 20, 20, 3, 3, "nc", 1'b0, 1, 115);
$rrc_watch("tb.separate", "recovery", "D", "any", "C", "posedge", 20, 20, "ns", 1'b0, 1, 115);
$rrc_watch("tb.separate", "removal", "D", "any", "C", "posedge", 8, 8, "nh", 1'b0, 1, 120);
$rrc_watch("tb.separate", "recrem", "D", "any", "C", "posedge", 20, 20, 8, 8, "nc", 1'b0, 2, 115, 120);
$rrc_watch("tb.wrong_edge", "recovery", "D", "any", "C", "posedge", 5, 5, "ns", 1'b0, 0);
$rrc_watch("tb.wrong_edge", "removal", "D", "any", "C", "posedge", 6, 6, "nh", 1'b0, 1, 120);
$rrc_watch("tb.wrong_edge", "recrem", "D", "any", "C", "posedge", 5, 5, 6, 6, "nc", 1'b0, 1, 120);
$rrc_watch("tb.wrong_data", "recovery", "D", "any", "C", "posedge", 5, 5, "ns", 1'b0, 0);
$rrc_watch("tb.wrong_data", "removal", "D", "any", "C", "posedge", 6, 6, "nh", 1'b0, 1, 120);
$rrc_watch("tb.wrong_data", "recrem", "D", "any", "C", "posedge", 5, 5, 6, 6, "nc", 1'b0, 1, 120);
$rrc_watch("tb.empty_recovery", "recovery", "D", "any", "C", "posedge", 5, 5, "ns", 1'b0, 0);
$rrc_watch("tb.empty_recovery", "removal", "D", "any", "C", "posedge", 8, 8, "nh", 1'b0, 1, 120);
$rrc_watch("tb.empty_recovery", "recrem", "D", "any", "C", "posedge", 5, 5, 8, 8, "nc", 1'b0, 1, 120);
$rrc_watch("tb.empty_removal", "recovery", "D", "any", "C", "posedge", 20, 20, "ns", 1'b0, 1, 115);
$rrc_watch("tb.empty_removal", "removal", "D", "any", "C", "posedge", 6, 6, "nh", 1'b0, 1, 120);
$rrc_watch("tb.empty_removal", "recrem", "D", "any", "C", "posedge", 20, 20, 6, 6, "nc", 1'b0, 2, 115, 120);
$rrc_start; #149; $rrc_finish; $finish; end
endmodule
