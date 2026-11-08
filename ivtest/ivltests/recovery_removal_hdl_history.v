// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module rec_history(R,C);
  input R,C;
  reg notifier; initial notifier=0;
  specify $recovery(posedge R,posedge C,10,notifier); endspecify
endmodule
`timescale 1ps/1ps
module rem_history(R,C);
  input R,C;
  reg notifier; initial notifier=0;
  specify $removal(posedge R,posedge C,10,notifier); endspecify
endmodule
`timescale 1ps/1ps
module tb;
  reg rz,cz,rl,cl,mrz,mcz,mrl,mcl;
  rec_history rz_u(rz,cz),rl_u(rl,cl);
  rem_history mz_u(mrz,mcz),ml_u(mrl,mcl);
  initial begin rz=1;cz=0;#3 cz=1;#1 cz=0;#1 cz=1;#1 cz=0;#14 cz=1;end
  initial begin cl=0;#20 cl=1;#1 cl=0;#79 rl=1;#9 cl=1;#1 cl=0;#10 cl=1;end
  initial begin mrz=0;mcz=1;#3 mrz=1;#1 mrz=0;#1 mrz=1;#1 mrz=0;#14 mrz=1;end
  initial begin mrl=0;#20 mrl=1;#1 mrl=0;#79 mcl=1;#9 mrl=1;end
  initial begin #1;
    $rr_watch("tb.rz_u", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "notifier", 2, 3, 5);
    $rr_watch("tb.rl_u", "recovery", "R", "posedge", "C", "posedge", 10.0, 10, "notifier", 1, 109);
    $rr_watch("tb.mz_u", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "notifier", 2, 3, 5);
    $rr_watch("tb.ml_u", "removal", "R", "posedge", "C", "posedge", 10.0, 10, "notifier", 1, 109);
    $rr_start;#139;$rr_finish;$finish;
  end
endmodule
