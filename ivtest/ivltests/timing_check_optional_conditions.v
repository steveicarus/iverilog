// Copyright (c) 2026 Stephen Williams (steve@icarus.com)
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ns/1ps
module ff(input CLK, D, RESET_B, stamp, check_en,
          output reg Q, n_sh, n_rr, n_setup);
  wire dclk, dd, dreset_b;
  initial begin n_sh = 0; n_rr = 0; n_setup = 0; end
  always @(posedge dclk or negedge dreset_b)
    if (!dreset_b) Q <= 0;
    else Q <= dd;
  specify
    $setuphold(posedge CLK, posedge D, 10, 10, n_sh, stamp,, dclk, dd);
    $setuphold(posedge CLK, negedge D, 10, 10, n_sh,, check_en, dclk, dd);
    $recrem(posedge RESET_B, posedge CLK, 10, 10, n_rr, stamp,, dreset_b, dclk);
    $recrem(posedge RESET_B, negedge CLK, 10, 10, n_rr,, check_en, dreset_b, dclk);
    $recrem(posedge RESET_B, posedge CLK, 10, 10, n_rr, stamp, check_en, dreset_b, dclk);
    $setup(posedge D, posedge CLK, 10, n_setup);
  endspecify
endmodule

module tb;
  reg clk, d, reset_b;
  wire q, n_sh, n_rr, n_setup;
  ff u(clk, d, reset_b, 1'b1, 1'b1, q, n_sh, n_rr, n_setup);
  initial begin
    clk = 0; d = 0; reset_b = 0;
    #1;
    $shc_population("tb.u", 1); // Only the ordinary $setup is executable.
    #19 reset_b = 1;
    #1 d = 1;
    #3 clk = 1;
    #1;
    if ({q, n_sh, n_rr, n_setup} !== 4'b1001) begin
      $display("FAILED sampling or timing notifier");
      $finish(0);
    end
    reset_b = 0;
    #1;
    if (q !== 0) begin
      $display("FAILED asynchronous reset");
      $finish(0);
    end
    $display("PASSED");
    $finish(0);
  end
endmodule
