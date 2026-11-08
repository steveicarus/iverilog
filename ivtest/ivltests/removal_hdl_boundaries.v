// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module rr_leaf(R,C);
  input R,C;
  reg notifier; initial notifier=0;
  specify $removal(posedge R,posedge C,10,notifier); endspecify
endmodule
`timescale 1ps/1ps
module tb;
  reg R,C;rr_leaf u(R,C);
  initial begin
    R=0;C=0;
    #20 R=1;#9 C=1;#1 begin R=0;C=0;end
    #20 R=1;#10 C=1;#1 begin R=0;C=0;end
    #19 R=1;#11 C=1;#1 begin R=0;C=0;end
    #18 C=1;#9 R=1;#1 begin R=0;C=0;end
    #20 C=1;#10 R=1;#1 begin R=0;C=0;end
    #19 C=1;#11 R=1;#1 begin R=0;C=0;end
    #18 R=1;#0 C=1;#1 begin R=0;C=0;end
    #19 C=1;#0 R=1;#1 begin R=0;C=0;end
    #19 R=1;#0 C=1;#0 R=0;#0 R=1;
  end
  initial begin
    #1;
    $rr_watch("tb.u", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "notifier", 1, 119);
    $rr_start;#259;$rr_finish;$finish;
  end
endmodule
