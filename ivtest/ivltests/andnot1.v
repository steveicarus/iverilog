`begin_keywords "1364-2005"
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

/* andnot1.v
 * This tests types.
 */
module main;

   reg a, b, c;

   wire d = a & !b;  // change from !b to ~b and everything is fine
   reg [2:0] tmp;
   reg	     ref;
   initial begin
      // Do an exaustive scan of the possible values.
      for (tmp = 0 ;  tmp < 4 ;  tmp = tmp + 1) begin
	 a = tmp[0];
	 b = tmp[1];
	 c = a & ~b;
	 #1 if (c != d) begin
	    $display("FAILED -- a=%b, b=%b, c=%b, d=%b",
		     a, b, c, d);
	    $finish;
	 end
      end // for (tmp = 0 ;  tmp < 4 ;  tmp = tmp + 1)

      $display("PASSED");
   end

endmodule // main
`end_keywords
