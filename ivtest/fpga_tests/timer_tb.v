/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
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
 *
 *   $Id: timer_tb.v,v 1.1 2003/04/01 05:55:24 stevewilliams Exp $
 */

`timescale 1us / 1us

module main;

   wire rdy;
   reg	reset, clk;

   timer dut(.rdy(rdy), .clk(clk), .reset(reset));

   always begin
      #5 clk = 1;
      #5 clk = 0;
   end

   initial begin
      $dumpvars(0, main);
      #7 reset = 1;
      #1 if (rdy !== 0) begin
	 $display("FAILED: reset did not clear rdy. rdy=%b", rdy);
	 $finish;
      end
      #6 reset = 0;
   end

   always @(posedge clk)
     if (rdy === 1) begin
	$display("rdy=%b at time=%0d", rdy,  $time);
	if ($time != 175) begin
	   $display("FAILED: timer ran out incorrectly.");
	   $finish;
	end

	$display("PASSED");
	$finish;
     end

endmodule // main
