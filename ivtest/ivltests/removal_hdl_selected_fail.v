// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module rr_nested(R,C);input R,C;endmodule
module tb(R,C);
 input R;input [1:0] C;reg n;
 specify $removal(posedge R,posedge C[0],10,n);endspecify
endmodule
