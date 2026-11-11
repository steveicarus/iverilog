// Copyright (c) 2026 Jerry Zhong
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module alias_pair(A, B, Y, Z);
  input [3:0] A;
  input [0:3] B;
  output [0:3] Y;
  output [7:4] Z;
  assign Y=A;
  assign Z=B;
endmodule
module alias_wrapper(A, B, Y, Z);
  input [3:0] A;
  input [0:3] B;
  output [0:3] Y;
  output [7:4] Z;
  alias_pair leaf(.Z(Z), .B(B), .Y(Y), .A(A));
endmodule
module tb;
  reg [3:0] A, B, T;
  wire [0:3] DY, TY, WY, WTY;
  wire [7:4] DZ, TZ, WZ, WTZ;
  // Distinct is emitted first; the later tied body must not be skipped.
  alias_pair a_distinct(A,B,DY,DZ);
  alias_pair z_tied(T,T,TY,TZ);
  alias_wrapper middle_distinct(A,B,WY,WZ);
  alias_wrapper middle_tied(T,T,WTY,WTZ);
  task check;
    input [3:0] a,b,t;
    begin
      A=a;B=b;T=t;#1;
      if ({DY,DZ,WY,WZ} !== {a,b,a,b}) $fatal(1,"FAILED distinct vector/nested");
      if ({TY,TZ,WTY,WTZ} !== {t,t,t,t}) $fatal(1,"FAILED tied vector/nested");
      if (DY[0]!==a[3] || DZ[4]!==b[0]) $fatal(1,"FAILED packed positions");
      if (middle_distinct.leaf.A!==a || middle_distinct.leaf.B!==b ||
          middle_tied.leaf.A!==t || middle_tied.leaf.B!==t)
        $fatal(1,"FAILED hierarchical source identity");
    end
  endtask
  initial begin
    check(4'b0011,4'b1100,4'b0000);
    check(4'b1010,4'b0101,4'b1111);
    check(4'bx10z,4'bz01x,4'bxzxz);
    check(4'bz01x,4'bx10z,4'bzxzx);
    // Per-instance specialization must retain native force/release behavior.
    check(4'b0000,4'b1111,4'b0101);
    force a_distinct.A=4'b1010; #1;
    if (DY!==4'b1010 || DZ!==4'b1111 || TY!==4'b0101)
      $fatal(1,"FAILED distinct force isolation");
    release a_distinct.A; #1;
    if (DY!==4'b0000) $fatal(1,"FAILED distinct release");
    force z_tied.B=4'b0011; #1;
    if (TY!==4'b0011 || TZ!==4'b0011 || DY!==4'b0000)
      $fatal(1,"FAILED tied force alias");
    release z_tied.B; #1;
    if (TY!==4'b0101 || TZ!==4'b0101) $fatal(1,"FAILED tied release");
    $display("PASSED");$finish;
  end
endmodule
