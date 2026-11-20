// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
// Fixed definition keeps SDF CELLTYPE stable through vlog95 translation.
`timescale 1ps/1ps
module shc_sdf_cell(C,D);
 input C,D; reg notifier; initial notifier=0;
 specify $setuphold(posedge C,D,10,6,notifier); endspecify
endmodule
