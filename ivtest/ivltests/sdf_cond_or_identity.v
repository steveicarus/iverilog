// Copyright (c) 2026 Jerry Zhong
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module or_identity_cell(input A,E,F, output Z);
  buf (Z,A);
  specify
    if ((E == 1'b1) || (F == 1'b1)) (A => Z) = (1,1);
    if ((E == 1'b1) && (F == 1'b1)) (A => Z) = (2,2);
  endspecify
endmodule
module top;
  reg A,E,F;
  wire Z;
  integer count;
  or_identity_cell u(A,E,F,Z);
  always @(Z) if ($time > 50) begin
    count = count+1;
    case (count)
      1: if (Z !== 1'b1 || $time != 107) $fatal(1,"FAIL OR rise %0t",$time);
      2: if (Z !== 1'b0 || $time != 209) $fatal(1,"FAIL OR fall %0t",$time);
      3: if (Z !== 1'b1 || $time != 302) $fatal(1,"FAIL AND rise %0t",$time);
      4: if (Z !== 1'b0 || $time != 402) $fatal(1,"FAIL AND fall %0t",$time);
      default: $fatal(1,"FAIL extra event");
    endcase
  end
  initial begin
    A=0;E=0;F=0;count=0;
    $sdf_annotate("ivltests/sdf_cond_or_identity.sdf",top);
    #10 E=1;
    #90 A=1;
    #100 A=0;
    #50 F=1;
    #50 A=1;
    #100 A=0;
    #100;
    if(count!=4) $fatal(1,"FAIL count %0d",count);
    $display("PASSED");
  end
endmodule
