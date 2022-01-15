/*
 * Copyright (c) 1998-2000 Stephen Williams (steve@icarus.com)
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
 * This test is inspired by (and tests) PR#12. The interesting aspect of
 * this is the multiply in the parameter passed to module foo instance yak.
 */
`define ONE 1
`define TWO 2

module foo(in,out);
   parameter blah = 2;

   input     in;
   output    out;

   initial begin
      if (blah != 4) begin
	 $display("FAILED -- parameter override of blah failed: %d", blah);
	 $finish;
      end

      $display("PASSED");
   end

endmodule

module bar;

	foo #(`ONE * 2 + `TWO) yak (,);

endmodule
