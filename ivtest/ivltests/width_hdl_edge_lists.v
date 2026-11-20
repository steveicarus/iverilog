// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2026 Jerry Zhong
// Equivalent directional edge lists retain WIDTH history and SDF identity.
`timescale 1ps/1ps
module width_edge_list_leaf(input CLK);
  reg kp=0, lp=0, ap=0, kn=0, ln=0, an=0;
  integer pk=0, pl=0, pa=0, nk=0, nl=0, na=0;
  always @(kp) if ($time > 0) pk=pk+1;
  always @(lp) if ($time > 0) pl=pl+1;
  always @(ap) if ($time > 0) pa=pa+1;
  always @(kn) if ($time > 0) nk=nk+1;
  always @(ln) if ($time > 0) nl=nl+1;
  always @(an) if ($time > 0) na=na+1;
  specify
    $width(posedge CLK, 10, 0, kp);
    $width(edge [01,0x,x1] CLK, 10, 0, lp);
    $width(edge [Z1,01,0Z] CLK, 10, 0, ap);
    $width(negedge CLK, 10, 0, kn);
    $width(edge [10,x0,1x] CLK, 10, 0, ln);
    $width(edge [1Z,10,Z0] CLK, 10, 0, an);
    $width(edge [01,0z,z1] CLK, 10);
    $width(edge [10,z0,1z] CLK, 10);
  endspecify
  task check;
    input integer positive, negative;
    begin
      if (pk !== positive || pl !== positive || pa !== positive ||
          nk !== negative || nl !== negative || na !== negative) begin
        $display("FAILED at %0t counts=%0d,%0d,%0d / %0d,%0d,%0d expected=%0d/%0d",
                 $time, pk, pl, pa, nk, nl, na, positive, negative);
        $finish(0);
      end
    end
  endtask
endmodule
module tb;
  reg CLK=0;
  width_edge_list_leaf dut(CLK);
  initial begin
    #1;
    $sdf_annotate("ivltests/width_hdl_edge_lists.sdf", dut);
    $width_edge_list_objects;
    #19 CLK=1; #1 dut.check(0,0);
    #3 CLK=1'bx; #1 dut.check(1,0);
    #3 CLK=1; #1 dut.check(1,1);
    #3 CLK=0; #1 dut.check(2,1);
    #3 CLK=1'bz; #1 dut.check(2,2);
    #3 CLK=1; #1 dut.check(2,3); // Retained negative opening checks again.
    #3 CLK=1'bz; #1 dut.check(3,3);
    #3 CLK=0; #1 dut.check(4,3); // Retained positive opening checks again.
    #3 CLK=1'bx; #1 dut.check(4,4);
    #3 CLK=1'bz; #1 dut.check(4,4);
    #3 CLK=1'bx; #1 dut.check(4,4);
    #3 CLK=0; #1 dut.check(4,4); // Positive width 12 equals its SDF limit.
    #13 CLK=1; #1 dut.check(4,4); // Negative width 14 equals its SDF limit.
    #11 CLK=0; #1 dut.check(4,4);
    $display("PASSED");
    $finish(0);
  end
endmodule
