// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module rrc_pos(C,D);
 parameter REC=10,REM=6; input C,D; reg notifier; initial notifier=0;
 specify $recrem(posedge D,posedge C,REC,REM,notifier); endspecify
endmodule
module rrc_any(C,D);
 parameter REC=10,REM=6; input C,D; reg notifier; initial notifier=0;
 specify $recrem(D,posedge C,REC,REM,notifier); endspecify
endmodule
