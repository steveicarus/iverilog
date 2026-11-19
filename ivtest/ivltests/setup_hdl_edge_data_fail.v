// Copyright (c) 2026 Jerry Zhong
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ns/1ps
module tb(input C, D);
  reg n;
  specify
    $setup(edge [01] D, posedge C, 10, n);
    $setup(edge [10, 0x, x1, 1x, x0] D, posedge C, 10, n);
    $setup(edge [0z, z1] D, posedge C, 10, n);
    $setup(edge [1z, z0] D, posedge C, 10, n);
    $setup(edge [01] D &&& C, posedge C, 10, n);
  endspecify
endmodule
