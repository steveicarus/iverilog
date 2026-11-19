// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module shc_precision(C,D);
 input C,D;
 specify $setuphold(posedge C,posedge D,1,1); endspecify
endmodule
module tb;
 reg C,D;
 shc_precision setup_only(C,D),hold_only(C,D);
 shc_precision omitted_setup(C,D),omitted_hold(C,D);
 initial begin
   C=0;D=0;
   #1;
   // Expected slots use exact integer words, never a floating-point oracle.
   $shc_exact_set("tb.setup_only",0,1,32'h00200000,1);
   $shc_exact_set("tb.hold_only",32'h00200000,3,0,1);
   $shc_exact_set("tb.omitted_setup",32'h00200000,5,0,1);
   $shc_exact_set("tb.omitted_hold",0,1,32'h00200000,7);
   $sdf_annotate("ivltests/setuphold_hdl_precision.sdf",tb);
   $shc_exact_expect("tb.setup_only",0,17,32'h00200000,1);
   $shc_exact_expect("tb.hold_only",32'h00200000,3,0,19);
   $shc_exact_expect("tb.omitted_setup",32'h00200000,5,0,23);
   $shc_exact_expect("tb.omitted_hold",0,29,32'h00200000,7);
   $shc_exact_finish;
   $finish;
 end
endmodule
