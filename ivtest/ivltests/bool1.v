/*
 * Copyright (c) 2005 Stephen Williams (steve@icarus.com)
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
 * Test the bool type used as an index to a for loop, and in a
 * few other minor cases.
 */
module main;

   reg bool [31:0] idx;
   reg logic [7:0] tmp;

   initial begin

      idx = 7;
      tmp = 7;
      $display("Dispay of 7s: %d, %d", idx, tmp);

      for (idx = 0 ;  idx < 17 ;  idx = idx + 1) begin
	 tmp = idx[7:0];
	 if (tmp != idx[7:0]) begin
	    $display("FAILED -- %b != %b", tmp, idx[7:0]);
	    $finish;
	 end

      end
      $display("PASSED");
   end

endmodule // main
