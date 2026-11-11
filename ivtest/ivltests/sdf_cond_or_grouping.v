// Copyright (c) 2026 Jerry Zhong
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module grouping_cell(input A,E,F,G, output L,R);
  buf (L,A), (R,A);
  specify
    if ((E == 1'b1) || ((F == 1'b1) && (G == 1'b1)))
      (A => L) = (1,1);
    if (((E == 1'b1) || (F == 1'b1)) && (G == 1'b1))
      (A => R) = (1,1);
  endspecify
endmodule
module top;
  reg A,E,F,G;
  wire L,R;
  integer lc,rc;
  grouping_cell u(A,E,F,G,L,R);
  task check;
    input integer n;
    input value;
    input integer t0,t1,t2,t3;
    begin
      case(n)
        1: if(value!==1'b1 || $time!=t0) $fatal(1,"FAIL first rise %0t",$time);
        2: if(value!==1'b0 || $time!=t1) $fatal(1,"FAIL first fall %0t",$time);
        3: if(value!==1'b1 || $time!=t2) $fatal(1,"FAIL second rise %0t",$time);
        4: if(value!==1'b0 || $time!=t3) $fatal(1,"FAIL second fall %0t",$time);
        default: $fatal(1,"FAIL extra event");
      endcase
    end
  endtask
  always @(L) if($time>50) begin lc=lc+1;check(lc,L,103,205,303,405);end
  always @(R) if($time>50) begin rc=rc+1;check(rc,R,107,209,300,400);end
  initial begin
    A=0;E=0;F=0;G=0;lc=0;rc=0;
    $sdf_annotate("ivltests/sdf_cond_or_grouping.sdf",top);
    #10 E=1;G=1;
    #90 A=1;
    #100 A=0;
    // E=1,F=0,G=0 distinguishes E||(F&&G) from (E||F)&&G.
    // No active path on R means zero path delay, not functional gating.
    #50 G=0;
    #50 A=1;
    #100 A=0;
    #50;
    if(lc!=4 || rc!=4) $fatal(1,"FAIL event counts %0d %0d",lc,rc);
    $display("PASSED");
  end
endmodule
