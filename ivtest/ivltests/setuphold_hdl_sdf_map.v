// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module shc_map(C,D,E);input C,D,E;reg ns,nh,nc;initial begin ns=0;nh=0;nc=0;end
 specify $setup(D,posedge C,5,ns);$hold(posedge C,D,6,nh);$setuphold(posedge C,D,5,6,nc);endspecify
endmodule
module tb;reg C,D,E;
shc_map empty_setup(C,D,E),empty_hold(C,D,E);
shc_map combined(C,D,E);
shc_map partial_setup(C,D,E);
shc_map partial_hold(C,D,E);
shc_map separate(C,D,E);
shc_map wrong_edge(C,D,E);
shc_map wrong_data(C,D,E);
initial begin C=0;D=0;E=0;#100 D=1;#15 C=1;#5 D=0;end
initial begin #1;
$sdf_annotate("ivltests/setuphold_hdl_map.sdf",tb);
$shc_population("tb.empty_setup", 3);
$shc_population("tb.empty_hold", 3);
$shc_population("tb.combined", 3);
$shc_population("tb.partial_setup", 3);
$shc_population("tb.partial_hold", 3);
$shc_population("tb.separate", 3);
$shc_population("tb.wrong_edge", 3);
$shc_population("tb.wrong_data", 3);
$shc_watch("tb.combined", "setup", "C", "posedge", "D", "any", 20, 20, "ns", 1, 115);
$shc_watch("tb.combined", "hold", "C", "posedge", "D", "any", 8, 8, "nh", 1, 120);
$shc_watch("tb.combined", "setuphold", "C", "posedge", "D", "any", 20, 20, 8, 8, "nc", 2, 115, 120);
$shc_watch("tb.partial_setup", "setup", "C", "posedge", "D", "any", 7, 7, "ns", 0);
$shc_watch("tb.partial_setup", "hold", "C", "posedge", "D", "any", 8, 8, "nh", 1, 120);
$shc_watch("tb.partial_setup", "setuphold", "C", "posedge", "D", "any", 7, 7, 8, 8, "nc", 1, 120);
$shc_watch("tb.partial_hold", "setup", "C", "posedge", "D", "any", 20, 20, "ns", 1, 115);
$shc_watch("tb.partial_hold", "hold", "C", "posedge", "D", "any", 3, 3, "nh", 0);
$shc_watch("tb.partial_hold", "setuphold", "C", "posedge", "D", "any", 20, 20, 3, 3, "nc", 1, 115);
$shc_watch("tb.separate", "setup", "C", "posedge", "D", "any", 20, 20, "ns", 1, 115);
$shc_watch("tb.separate", "hold", "C", "posedge", "D", "any", 8, 8, "nh", 1, 120);
$shc_watch("tb.separate", "setuphold", "C", "posedge", "D", "any", 20, 20, 8, 8, "nc", 2, 115, 120);
$shc_watch("tb.wrong_edge", "setup", "C", "posedge", "D", "any", 5, 5, "ns", 0);
$shc_watch("tb.wrong_edge", "hold", "C", "posedge", "D", "any", 6, 6, "nh", 1, 120);
$shc_watch("tb.wrong_edge", "setuphold", "C", "posedge", "D", "any", 5, 5, 6, 6, "nc", 1, 120);
$shc_watch("tb.wrong_data", "setup", "C", "posedge", "D", "any", 5, 5, "ns", 0);
$shc_watch("tb.wrong_data", "hold", "C", "posedge", "D", "any", 6, 6, "nh", 1, 120);
$shc_watch("tb.wrong_data", "setuphold", "C", "posedge", "D", "any", 5, 5, 6, 6, "nc", 1, 120);
$shc_watch("tb.empty_setup","setup","C","posedge","D","any",5,5,"ns",0);
$shc_watch("tb.empty_setup","hold","C","posedge","D","any",8,8,"nh",1,120);
$shc_watch("tb.empty_setup","setuphold","C","posedge","D","any",5,5,8,8,"nc",1,120);
$shc_watch("tb.empty_hold","setup","C","posedge","D","any",20,20,"ns",1,115);
$shc_watch("tb.empty_hold","hold","C","posedge","D","any",6,6,"nh",1,120);
$shc_watch("tb.empty_hold","setuphold","C","posedge","D","any",20,20,6,6,"nc",2,115,120);
$shc_start; #149; $shc_finish; $finish; end
endmodule
