// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module shc_disabled(C,D,DC,DD);input C,D;output DC,DD;wire DC,DD;
 specify $setuphold(posedge C,D,-1,2,,,,DC,DD);endspecify endmodule
module tb;reg C,D;wire DC,DD;shc_disabled u(C,D,DC,DD);
 initial begin C=0;D=0;#1;if(DC!==C || DD!==D) begin $display("FAILED initial delayed copies");$finish;end
 #10 C=1;D=1;#1;if(DC!==C || DD!==D) begin $display("FAILED delayed copies changed");$finish;end
 $display("PASSED");$finish;end endmodule
