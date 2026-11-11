// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module sh_nested(D);input D;endmodule
module tb(C,D);
 input C;input D;reg n;
 specify $setup(D,posedge C,-1,n);endspecify
endmodule
