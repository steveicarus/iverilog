// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module shared_input(input clk, d, output reg q);
  wire dc, dd1, dd2;
  reg notifier;
  initial notifier = 0;
  always @(posedge dc) q <= dd1;
  specify
    $setuphold(posedge clk, posedge d, 116, -62, notifier,,, dc, dd1);
    $setuphold(posedge clk, negedge d, 0, 0,,,,, dd2);
  endspecify
endmodule

module tb;
  reg raw;
  wire q;
  shared_input u(raw, raw, q);
  initial begin
    raw = 0;
    #100;
    force u.dd1 = 1;
    #1;
    if ({raw, u.dd1, u.dd2} !== 3'b010) begin
      $display("FAILED force crossed named delayed terminals");
      $finish;
    end
    release u.dd1;
    #1;
    if ({raw, u.dd1, u.dd2} !== 3'b000) begin
      $display("FAILED release did not restore delayed D");
      $finish;
    end
    #1898 raw = 1;
    #1;
    // Raw clock and data are tied, but the checked data arrives at 2063 ps.
    if ({q, u.dd1, u.dd2, u.notifier} !== 4'b0000) begin
      $display("FAILED simultaneous raw event collapsed delayed data");
      $finish;
    end
    #63;
    if ({q, u.dd1, u.dd2, u.notifier} !== 4'b0110)
      $display("FAILED signed delayed delivery");
    else
      $display("PASSED");
    $finish;
  end
endmodule
