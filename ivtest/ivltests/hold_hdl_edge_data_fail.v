// Copyright (c) 2026 Jerry Zhong
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ns/1ps
module tb(input C, D);
  reg n;
  specify
    $hold(posedge C, edge [01] D, 10, n);
    $hold(posedge C, edge [10, 0x, x1, 1x, x0] D, 10, n);
    $hold(posedge C, edge [0z, z1] D, 10, n);
    $hold(posedge C, edge [1z, z0] D, 10, n);
    $hold(posedge C, edge [01] D &&& C, 10, n);
  endspecify
endmodule
