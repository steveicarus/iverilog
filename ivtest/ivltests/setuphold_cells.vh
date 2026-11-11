// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module shc_pos(C,D);
 parameter S=10,H=6; input C,D; reg notifier; initial notifier=0;
 specify $setuphold(posedge C,posedge D,S,H,notifier); endspecify
endmodule
module shc_any(C,D);
 parameter S=10,H=6; input C,D; reg notifier; initial notifier=0;
 specify $setuphold(posedge C,D,S,H,notifier); endspecify
endmodule
