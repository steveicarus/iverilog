/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
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
 *   $Id: sqrt.v,v 1.1 2003/03/30 03:54:48 stevewilliams Exp $"
 */

 /*
  * This module approximates the square root of an unsigned 32bit
  * number. The algorithm works by doing a bit-wise binary search.
  * Starting from the most significant bit, the accumulated value
  * tries to put a 1 in the bit position. If that makes the square
  * too big for the input, the bit is left zero, otherwise it is set
  * in the result. This continues for each bit, decreasing in
  * significance, until all the bits are calculated or all the
  * remaining bits are zero.
  *
  * Since the result is an integer, this function really calculates
  * value of the expression:
  *
  *      x = floor(sqrt(y))
  *
  * where sqrt(y) is the exact square root of y and floor(N) is the
  * largest integer <= N.
  *
  * For 32bit numbers, this will never run more then 16 iterations,
  * which amounts to 16 clocks.
  */

module sqrt32(clk, rdy, reset, x, .y(acc));
   input  clk;
   output rdy;
   input  reset;

   input [31:0] x;
   output [15:0] acc;


   // acc holds the accumulated result, and acc2 is the accumulated
   // square of the accumulated result.
   reg [15:0] acc;
   reg [31:0] acc2;

   // Keep track of which bit I'm working on.
   reg [4:0]  bitl;
   wire [15:0] bit = 1 << bitl;
   wire [31:0] bit2 = 1 << (bitl << 1);

   // The output is ready when the bitl counter underflows.
   wire rdy = bitl[4];

   // guess holds the potential next values for acc, and guess2 holds
   // the square of that guess. The guess2 calculation is a little bit
   // subtle. The idea is that:
   //
   //      guess2 = (acc + bit) * (acc + bit)
   //             = (acc * acc) + 2*acc*bit + bit*bit
   //             = acc2 + 2*acc*bit + bit2
   //             = acc2 + 2 * (acc<<bitl) + bit
   //
   // This works out using shifts because bit and bit2 are known to
   // have only a single bit in them.
   wire [15:0] guess  = acc | bit;
   wire [31:0] guess2 = acc2 + bit2 + ((acc << bitl) << 1);

   (* ivl_synthesis_on *)
   always @(posedge clk or posedge reset)
      if (reset) begin
	 acc = 0;
	 acc2 = 0;
	 bitl = 15;
      end else begin
	 if (guess2 <= x) begin
	    acc  <= guess;
	    acc2 <= guess2;
	 end
	 bitl <= bitl - 5'd1;
      end

endmodule // sqrt32

 /*
  * This module represents the chip packaging that we intend to
  * generate. We bind pins here, and route the clock to the global
  * clock buffer.
  */
module chip_root(clk, rdy, reset, x, y);
   input  clk;
   output rdy;
   input  reset;

   input [31:0] x;
   output [15:0] y;

   wire		 clk_int;

   (* cellref="BUFG:O,I" *)
   buf gbuf (clk_int, clk);

   sqrt32 dut(.clk(clk_int), .reset(reset), .rdy(rdy), .x(x), .y(y));

   /* Assign the clk to GCLK0, which is on pin P39. */
   $attribute(clk, "PAD", "39");

   // We don't care where the remaining pins go, so set the pin number
   // to 0. This tells the implementation tools that we want a PAD,
   // but we don't care which. Also note the use of a comma (,)
   // separated list to assign pins to the bits of a vector.
   $attribute(rdy, "PAD", "0");
   $attribute(reset, "PAD", "0");
   $attribute(x, "PAD", "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
   $attribute(y, "PAD", "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");

endmodule // chip_root
