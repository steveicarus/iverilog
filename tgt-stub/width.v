// Copyright (c) 2026 Jerry Zhong
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ns/1ps
module width_stub_leaf(input wire CLK, input wire GATE);
  reg notifier;
  specify
    $width(posedge CLK, 1.25, 0.125, notifier);
    $width(negedge GATE, 2.5);
  endspecify
endmodule
module top;
  reg clk = 0;
  reg gate = 1;
  width_stub_leaf a(clk, gate);
  width_stub_leaf b(clk, gate);
endmodule
