// Copyright (c) 2026 Jerry Zhong
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ps/1ps
module or_tree_cell(input A,E,F,G,H, output Z0,Z1,Z2,Z3,Z4,Z5,Z6,Z7);
  wire \E.dot = H;
  buf (Z0,A), (Z1,A), (Z2,A), (Z3,A), (Z4,A), (Z5,A), (Z6,A), (Z7,A);
  specify
    if ((E == 1'b1) || (F == 1'b1)) (A => Z0) = (1,1);
    if (((E == 1'b1) || (F == 1'b1)) || (G == 1'b1)) (A => Z1) = (1,1);
    if ((E == 1'b1) || ((F == 1'b1) || (G == 1'b1))) (A => Z2) = (1,1);
    if ((E == 1'b1) || ((F == 1'b1) && (G == 1'b1))) (A => Z3) = (1,1);
    if (((E == 1'b1) || (F == 1'b1)) && (G == 1'b1)) (A => Z4) = (1,1);
    if ((E == 1'b1) && ((F == 1'b1) || (G == 1'b1))) (A => Z5) = (1,1);
    if (((E == 1'b1) && (F == 1'b1)) || (G == 1'b1)) (A => Z6) = (1,1);
    if ((\E.dot == 1'b1) || (F == 1'b0)) (A => Z7) = (1,1);
  endspecify
endmodule
module top;
  reg A,E,F,G,H;
  wire z0,z1,z2,z3,z4,z5,z6,z7;
  reg [31:0] count[0:7];
  integer i;
  or_tree_cell u(A,E,F,G,H,z0,z1,z2,z3,z4,z5,z6,z7);
  task check;
    input integer index;
    input value;
    input integer rise1, fall1, rise2, fall2;
    begin
      if ($time > 50) begin
        count[index] = count[index]+1;
        case (count[index])
          1: if (value !== 1'b1 || $time != rise1) $fatal(1,"FAIL rise1 %0d %0t",index,$time);
          2: if (value !== 1'b0 || $time != fall1) $fatal(1,"FAIL fall1 %0d %0t",index,$time);
          3: if (value !== 1'b1 || $time != rise2) $fatal(1,"FAIL rise2 %0d %0t",index,$time);
          4: if (value !== 1'b0 || $time != fall2) $fatal(1,"FAIL fall2 %0d %0t",index,$time);
          default: $fatal(1,"FAIL extra event %0d",index);
        endcase
      end
    end
  endtask
  always @(z0) check(0,z0,103,205,313,405);
  always @(z1) check(1,z1,107,209,307,409);
  always @(z2) check(2,z2,111,213,311,413);
  always @(z3) check(3,z3,115,217,315,417);
  always @(z4) check(4,z4,119,221,319,421);
  always @(z5) check(5,z5,123,225,323,425);
  always @(z6) check(6,z6,127,229,327,429);
  always @(z7) check(7,z7,131,233,331,433);
  initial begin
    A=0;E=0;F=0;G=0;H=0;
    for(i=0;i<8;i=i+1) count[i]=0;
    $sdf_annotate("ivltests/sdf_cond_or_tree.sdf",top);
    #10;E=1;G=1;H=1;
    #90 A=1;
    #100 A=0;
    #50 $sdf_annotate("ivltests/sdf_cond_or_tree_late.sdf",top);
    #50 A=1;
    #100 A=0;
    #100;
    for(i=0;i<8;i=i+1) if(count[i]!=4) $fatal(1,"FAIL count %0d %0d",i,count[i]);
    $display("PASSED");
  end
endmodule
