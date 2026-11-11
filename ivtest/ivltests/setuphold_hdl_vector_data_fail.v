// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module shc_nested(D);input D;endmodule
module tb(C,D);
 input C;input [1:0] D;reg n;
 specify $setuphold(posedge C,D,10,6,n);endspecify
endmodule
