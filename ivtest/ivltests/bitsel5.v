/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
 * Bit select of a net (a wire) using the index of a for loop.
 */

module main;

   // Make a vector of bits, an array of functors in practice, and
   // create a net that hooks to that array backwards.
   reg [5:1]  vect = 5'b10100;
   wire [5:1] tmp = { vect[1], vect[2], vect[3], vect[4], vect[5] };

   reg [2:0]  idx;
   initial begin
      #1 $display("vect=%b, tmp=%b", vect, tmp);

      for (idx = 1 ;  idx <= 5 ;  idx = idx + 1) begin
	 $display("idx=%d: vect=%b, tmp=%b", idx, vect[idx], tmp[idx]);
	 if (tmp[idx] !== vect[6-idx]) begin
	    $display("FAILED");
	    $finish;
	 end
      end

      $display("PASSED");
   end

endmodule // main
