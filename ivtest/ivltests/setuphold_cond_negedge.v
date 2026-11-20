// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module neg(C,D,R,E);input C,D,R,E;
 specify $setuphold(negedge C &&& R,negedge D &&& E,10,10);endspecify
endmodule
module tb;
reg C,D,R,E;neg u(C,D,R,E);initial begin C=1;D=1;R=0;E=0;
#50 D=0;#5 C=0;#5 C=1;D=1;#20 R=1;E=1;#20 D=0;#5 C=0;
#15 C=1;D=1;#80 C=0;#5 D=0;end
initial begin #1;
$shc_population("tb.u",1);
$shc_watch("tb.u","setuphold","C","negedge","D","negedge",10,10,10,10,"",1'b0,2,105,205);
$shc_start; #229; $shc_finish; $finish; end
endmodule
