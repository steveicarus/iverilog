//                              -*- Mode: Verilog -*-
// Filename        : cnr_tb.v
// Description     : single row corner bender testbench
// Author          :
// Created On      : Thu Mar 23 16:23:01 2006
// Last Modified By: $Id: pr1492075.v,v 1.1 2006/06/02 05:01:52 stevewilliams Exp $
// Last Modified On: .
// Status          : Unknown, Use with caution!

`timescale 1ns / 10ps

module cnr_tb ();

   reg clkb;
   reg clocken;
   integer  cntb;

   // clock generation clkb
   always @ (posedge clocken)
     begin
	for (cntb=0; cntb<5; cntb=cntb+1)
	  begin
	     #(10 + -2) clkb = 1;
	     #(10 - -2) clkb = 0;
	  end
     end

   //
   initial
     begin
	$monitor("clkb=%b at %t", clkb, $time);
	clkb = 1'b0;
	clocken = 0;
	#1 clocken = 1;
	#(10*20) clocken = 0;

	#100;
	$finish(0);
     end

endmodule // cnr_tb
