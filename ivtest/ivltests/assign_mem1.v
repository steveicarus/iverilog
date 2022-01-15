/*
 * Copyright (c) 2000 Chris Lattner
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

/*
 * This isn't computationally complicated, but can trip up a vvm
 * code generation error.
 */
module test;
   reg [15:0] is[1:0];
   reg [4:0]  i;

   initial begin
      i = 0;
      is[0] = i; // Notice the different widths.
      if (is[0] !== 16'd0) begin
	 $display("FAILED -- is[0] --> %b", is[0]);
	 $finish;
      end
      $display("PASSED");
   end
endmodule
