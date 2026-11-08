// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module rr_alias_rec(R,C);
  input R,C;
  reg notifier; initial notifier=0;
  specify $recovery(posedge R,posedge C,10,notifier); endspecify
endmodule
`timescale 1ps/1ps
module rr_alias_rem(R,C);
  input R,C;
  reg notifier; initial notifier=0;
  specify $removal(posedge R,posedge C,10,notifier); endspecify
endmodule
`timescale 1ps/1ps
module rr_same(S);
 input S;reg nr,nm;initial begin nr=0;nm=0;end
 specify $recovery(posedge S,posedge S,10,nr);$removal(posedge S,posedge S,10,nm);endspecify
endmodule
module tb;
 reg R,C,reverseR,reverseC,S;
 rr_alias_rec ordered_r(R,C),reverse_r(reverseR,reverseC),alias_r(S,S);
 rr_alias_rem ordered_m(R,C),reverse_m(reverseR,reverseC),alias_m(S,S);
 rr_same same(S);
 initial begin R=0;C=0;#100 R=1;#0 C=1;end
 initial begin reverseR=0;reverseC=0;#100 reverseC=1;#0 reverseR=1;end
 initial begin S=0;#100 S=1;#1 S=0;#99 S=1;end
 initial begin #1;
    $rr_watch("tb.ordered_r", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "notifier", 1, 100);
    $rr_watch("tb.ordered_m", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "notifier", 0);
    $rr_watch("tb.reverse_r", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "notifier", 0);
    $rr_watch("tb.reverse_m", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "notifier", 0);
    $rr_watch("tb.alias_r", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "notifier", 2, 100, 200);
    $rr_watch("tb.alias_m", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "notifier", 0);
    $rr_watch("tb.same", "recovery", "S", "posedge", "S", "posedge", 10.0, 10, "nr", 2, 100, 200);
    $rr_watch("tb.same", "removal", "S", "posedge", "S", "posedge", 10.0, 10, "nm", 0);
  $rr_start;#219;$rr_finish;$finish;
 end
endmodule
