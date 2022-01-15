/*
 * Copyright (c) 2000 Intrinsity, Inc.
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
module rpull ( i, o);
input i;
output o;

wire gnd;
wire vdd;
wire pu0;
wire pu1;

reg failed;

assign gnd = 1'b0;
assign vdd = 1'b1;

assign (pull0,pull1) pu0 = 1'b0;
assign (pull0,pull1) pu1 = 1'b1;

rnmos n0 ( o,  gnd,  i);
rpmos p0 ( o,  vdd,  i);

initial begin
#1;
  failed = 0;
  if (i === vdd)
    if (o !== pu0) begin
      $display ("FAILED: test_mos_strength_reduction:  case pull i:%d o:%d pu0:%d", i, o, pu0);
      failed = 1;
    end

  else if (i === gnd)
    if (o !== pu1) begin
      $display ("FAILED: test_mos_strength_reduction:  case pull i:%d o:%d pu0:%d", i, o, pu0);
      failed = 1;
    end
  else begin
    $display ("FAILED: test_mos_strength_reduction:  case pull i:%d o:%d pu0:%d", i, o, pu0);
      failed = 1;
  end

  if ( ! failed )
    $display ("PASSED");
end
endmodule

module rweak (i, o);
input i;
output o;

wire gnd;
wire vdd;
wire we0;
wire we1;

reg failed;

assign gnd = 1'b0;
assign vdd = 1'b1;

assign (weak0,weak1) we0 = 1'b0;
assign (weak0,weak1) we1 = 1'b1;

rnmos rn0 ( n0, gnd, i);
rnmos rn1 (  o,  n0, i);
rpmos rp1 (  o,  p0, i);
rpmos rp0 ( p0, vdd, i);

initial begin
#1;
  failed = 0;
  if (i === vdd)
    if (o !== we0) begin
      $display ("FAILED: test_mos_strength_reduction:  case weak i:%d o:%d pu0:%d", i, o, we0);
      failed = 1;
    end

  else if (i === gnd)
    if (o !== we1) begin
      $display ("FAILED: test_mos_strength_reduction:  case weak i:%d o:%d pu0:%d", i, o, we0);
      failed = 1;
    end
  else begin
    $display ("FAILED: test_mos_strength_reduction:  case weak i:%d o:%d pu0:%d", i, o, we0);
     failed = 1;
  end

  if ( ! failed )
     $display ("PASSED: test_mos_strength_reduction:  case rweak");
end
endmodule
module test_mos_strength_reduction;	/* beginning of _testbench */

reg vdd;
reg gnd;

reg c0,c1;
reg failed;

wire n0,p0;
wire n1,p1;
wire n2,p2;
wire n3,p3;
wire n4,p4;

wire st1st0;
wire pu1pu0;
wire we1pu0;
wire me1pu0;
wire sm1pu0;

wire o0;
wire o1;

assign (strong1, strong0) st1st0 = 1'b1;
assign (strong1, strong0) st1st0 = 1'b0;

assign (pull1, pull0) pu1pu0 = 1'b1;
assign (pull1, pull0) pu1pu0 = 1'b0;

assign (weak1, weak0) we1pu0 = 1'b1;
assign (pull1, pull0) we1pu0 = 1'b0;

rpull pu0 (vdd,o0);
rweak we0 (vdd,o1);

rnmos rn_0 (n1,gnd,c0);
rnmos rn_1 (n2,n1,c0);
rnmos rn_2 (n3,n2,c0);
rnmos rn_3 (n4,n3,c0);
rnmos rn_4 ( o,n4,c0);
rpmos rp_0 (p0,vdd,c1);
rpmos rp_1 (p1,p0,c1);
rpmos rp_2 (p2,p1,c1);
rpmos rp_3 (p3,p2,c1);
rpmos rp_4 ( o,p3,c1);


initial begin
  failed = 0;
  vdd = 1'b1;
  gnd = 1'b0;
#1;
  c0 = 1'b1;
  c1 = 1'b1;
#1;
  if (o !== gnd ) begin
    $display ("FAILED: test_mos_strength_reduction:  case 0");
    failed = 1;
  end
#1;
  c0 = 1'b0;
  c1 = 1'b0;
#1;
  if (o !== vdd ) begin
    $display ("FAILED: test_mos_strength_reduction:  case 1");
    failed = 1;
  end
#1;
  c0 = 1'b1;
  c1 = 1'b0;
#1;
  if (o !== 1'bx ) begin
    $display ("FAILED: test_mos_strength_reduction:  case x");
    failed = 1;
  end
  if (! failed )
    $display ("PASSED: test_mos_strength_reduction");
#1;
end
endmodule
