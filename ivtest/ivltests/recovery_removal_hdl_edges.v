// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module rec_neg(R,C);
  input R,C;
  reg notifier; initial notifier=0;
  specify $recovery(negedge R,negedge C,10,notifier); endspecify
endmodule
`timescale 1ps/1ps
module rem_neg(R,C);
  input R,C;
  reg notifier; initial notifier=0;
  specify $removal(negedge R,negedge C,10,notifier); endspecify
endmodule
`timescale 1ps/1ps
module rec_any(R,C);
  input R,C;
  reg notifier; initial notifier=0;
  specify $recovery(R,C,10,notifier); endspecify
endmodule
`timescale 1ps/1ps
module rem_any(R,C);
  input R,C;
  reg notifier; initial notifier=0;
  specify $removal(R,C,10,notifier); endspecify
endmodule
`timescale 1ps/1ps
module tb;
  reg R,C;rec_neg rn(R,C);rem_neg mn(R,C);rec_any ra(R,C);rem_any ma(R,C);
  initial begin R=1;C=1;
    #20 R=0;#9 C=0;#1 R=1;#10 C=1;#5 R=0;
    #15 C=0;#5 R=1;#5 R=0;#10 C=1;#5 C=0;#5 R=1;#4 R=0;
  end
  initial begin #1;
    $rr_watch("tb.rn", "recovery", "R", "negedge", "C", "negedge", 10.0, 10, "notifier", 1'b0, 1, 29);
    $rr_watch("tb.mn", "removal", "R", "negedge", "C", "negedge", 10.0, 10, "notifier", 1'b0, 1, 94);
    // R is assigned before C at t0: any-edge recovery sees interval zero.
    $rr_watch("tb.ra", "recovery", "R", "any", "C", "any", 10.0, 10, "notifier", 1'b0, 2, 0, 29);
    $rr_watch("tb.ma", "removal", "R", "any", "C", "any", 10.0, 10, "notifier", 1'b0, 5, 30, 45, 65, 90, 94);
    $rr_start;#109;$rr_finish;$finish;
  end
endmodule
