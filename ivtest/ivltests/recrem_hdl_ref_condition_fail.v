// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module rrc_nested(D);input D;endmodule
module tb(C,D);
 input C,D;reg n;wire EN=1;
 specify $recrem(posedge C &&& (EN && EN),D,10,6,n);endspecify
endmodule
