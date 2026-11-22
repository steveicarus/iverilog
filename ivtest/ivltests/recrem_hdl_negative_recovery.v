// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module tb;
   reg C, D, notifier;

   specify
      $recrem(posedge C,D, -1, 8, notifier);
   endspecify
   initial begin
      C = 0; D = 0; notifier = 0; #1;
      $rrc_population("tb", 1);
      $rrc_watch("tb", "recrem", "C", "posedge", "D", "any",
                 0, 0, 8, 8, "notifier", 1'b0, 0);
      $rrc_start;
      #1; $rrc_finish;
      $finish(0);
   end
endmodule
