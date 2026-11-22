// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module tb;
   reg C, D, notifier;
   wire DD;
   specify
      $setuphold(posedge C,D, 0, 0, notifier,,,, DD);
   endspecify
   initial begin
      C = 0; D = 0; #1;
      if (DD !== D) begin $display("FAILED initial copy"); $finish; end
      D = 1; #5; C = 1; #1;
      if (DD !== D) begin $display("FAILED rising copy"); $finish; end
      C = 0; D = 0; #1;
      if (DD !== D) begin $display("FAILED falling copy"); $finish; end
      $display("PASSED");
      $finish;
   end
endmodule
