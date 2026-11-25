// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module tb;
   reg C, D, notifier;

   specify
      $setuphold(posedge C,D, 10, -1, notifier);
   endspecify
   initial begin
      C = 0; D = 0; notifier = 0; #1;
      $shc_population("tb", 1);
      $shc_watch("tb", "setuphold", "C", "posedge", "D", "any",
                 10, 10, -1, 0, "notifier", 1'b0, 0);
      $shc_start;
      #1; $shc_finish;
      $finish(0);
   end
endmodule
