/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
 * This program is designed to test non-constant bit selects in the
 * concatenated l-value of procedural assignment.
 */
module main;

   reg [3:0] vec;
   reg	     a;
   integer   i;

   initial begin
      vec = 4'b0000;
      a = 0;

      if (vec !== 4'b0000) begin
	 $display("FAILED -- initialized vec to %b", vec);
	 $finish;
      end

      for (i = 0 ;  i < 4 ;  i = i + 1) begin
	 { a, vec[i] } = 2'b11;
      end

      if (vec !== 4'b1111) begin
	 $display("FAILED == vec (%b) is not 1111", vec);
	 $finish;
      end

      if (a !== 1'b1) begin
	 $display("FAILED -- a (%b) is not 1", a);
	 $finish;
      end

      $display("PASSED");
   end // initial begin
endmodule // main
