// Copyright (c) 2026 Jerry Zhong
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module or_reject_cell(input A,E,F,G,H, output Z,ZL);
  buf (Z,A), (ZL,A);
  specify
    if ((E == 1'b1) || (F == 1'b1)) (A => Z) = (1,1);
    if (((E == 1'b1) || (F == 1'b1)) || (G == 1'b1)) (A => ZL) = (1,1);
  endspecify
endmodule
module top;
  reg A,E,F,G,H;
  wire zb,zl,zg,zunused;
  integer cb,cl,cg;
  or_reject_cell untouched(A,E,F,G,H,zb,zl);
  or_reject_cell updated(A,E,F,G,H,zg,zunused);
  always @(zb) if($time>50) begin
    cb=cb+1;
    if(!((cb==1 && zb===1'b1 && $time==101) ||
         (cb==2 && zb===1'b0 && $time==201))) $fatal(1,"FAIL rejected OR %0t",$time);
  end
  always @(zl) if($time>50) begin
    cl=cl+1;
    if(!((cl==1 && zl===1'b1 && $time==101) ||
         (cl==2 && zl===1'b0 && $time==201))) $fatal(1,"FAIL rejected tree %0t",$time);
  end
  always @(zg) if($time>50) begin
    cg=cg+1;
    if(!((cg==1 && zg===1'b1 && $time==107) ||
         (cg==2 && zg===1'b0 && $time==209))) $fatal(1,"FAIL later valid CELL %0t",$time);
  end
  initial begin
    A=0;E=0;F=0;G=0;H=0;cb=0;cl=0;cg=0;
    $sdf_annotate("ivltests/sdf_cond_or_reject.sdf",top);
    #10;E=1;G=1;H=1;
    #90 A=1;
    #100 A=0;
    #100;
    if(cb!=2 || cl!=2 || cg!=2) $fatal(1,"FAIL counts %0d %0d %0d",cb,cl,cg);
    $display("PASSED");
  end
endmodule
