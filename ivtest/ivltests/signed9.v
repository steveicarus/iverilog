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
 */

module assignsigned();
   parameter foo = 10;
   reg	     signed [15:0] bar = -1;
   wire      baz;
   assign    baz = (bar < $signed(foo));

   initial begin
      #1 $display("bar=%h(%0d), foo=%0d, baz = %b", bar, bar, foo, baz);
      if (baz !== 1'b1) begin
	 $display("FAILED -- Compare returns %b instead of 1.", baz);
	 $finish;
      end
      $display("PASSED");
   end

endmodule
