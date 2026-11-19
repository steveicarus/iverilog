// Copyright (c) 2026 Jerry Zhong
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ns/1ps
module tb(input C, D);
  reg n;
  specify
    $setup(D, edge [01] C, 10, n);
    $setup(D, edge [10, 0x, x1, 1x, x0] C, 10, n);
    $setup(D, edge [0z, z1] C, 10, n);
    $setup(D, edge [1z, z0] C, 10, n);
    $setup(D, edge [01] C &&& D, 10, n);
  endspecify
endmodule
