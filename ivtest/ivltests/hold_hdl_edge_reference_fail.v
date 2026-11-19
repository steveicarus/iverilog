// Copyright (c) 2026 Jerry Zhong
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ns/1ps
module tb(input C, D);
  reg n;
  specify
    $hold(edge [01] C, D, 10, n);
    $hold(edge [10, 0x, x1, 1x, x0] C, D, 10, n);
    $hold(edge [0z, z1] C, D, 10, n);
    $hold(edge [1z, z0] C, D, 10, n);
    $hold(edge [01] C &&& D, D, 10, n);
  endspecify
endmodule
