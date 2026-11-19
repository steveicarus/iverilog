// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2026 Jerry Zhong
module tb;
  reg CLK, EN;
  specify
    $width(edge [01] CLK, 10);
    $width(edge [01,10] CLK, 10);
    $width(edge [01,0x,x1] CLK &&& EN, 10);
  endspecify
endmodule
